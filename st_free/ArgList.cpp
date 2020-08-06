#include "include/ArgList.hpp"

namespace ST_free {
bool ArgStatus::isMemberFreed(std::queue<int> indexes) {
  if (indexes.size() > 0) {
    int topIndex = indexes.front();
    indexes.pop();

    if (topIndex < structStatus.size())
      return structStatus[topIndex].isMemberFreed(indexes);
    else
      return false;
  }
  return this->isFreed();
}

void ArgStatus::setMemberFreed(std::queue<int> indexes) {
  if (indexes.size() > 0) {
    int topIndex = indexes.front();
    indexes.pop();

    if (topIndex >= this->maxSize()) return;

    if (topIndex >= this->size())
      this->extendStatusSize(topIndex - this->size() + 1);
    if (structStatus.size() > topIndex)
      structStatus[topIndex].setMemberFreed(indexes);
    return;
  }

  this->setFreed();
  return;
}

int ArgStatus::maxSize() {
  if (this->isStruct())
    return get_type(this->getType())->getStructNumElements();
  return 0;
}

void ArgStatus::extendStatusSize(int extSize) {
  for (int i = 0; i < extSize && this->size() < this->maxSize();
       i++, statusSize++) {
    llvm::Type *memTy = get_type(T)->getStructElementType(this->size());
    structStatus.push_back(ArgStatus(memTy));
  }
}

std::vector<bool> ArgStatus::getFreedList() {
  std::vector<bool> freedList;
  for (auto member : structStatus) {
    freedList.push_back(member.isFreed());
  }
  return freedList;
}

void ArgList::setArg(uint64_t arg_no, llvm::Value *V) {
  if (arg_no < arg_list.size()) {
    arg_list[arg_no] = V;
    if (auto alloca_inst = llvm::dyn_cast<llvm::AllocaInst>(V))
      stats[arg_no].setType(alloca_inst->getAllocatedType());
    else
      stats[arg_no].setType(V->getType());
  }
  return;
}

llvm::Value *ArgList::getArg(uint64_t arg_no) {
  if (arg_no < arg_list.size()) {
    return arg_list[arg_no];
  }
  return NULL;
}

void ArgList::setArgs(llvm::Function *F) {
  int i = 0;
  for (auto args = F->arg_begin(); args != F->arg_end(); args++, i++) {
    llvm::Value *v = getArgAlloca(args);
    if (!v) v = args;
    if (v) this->setArg(i, v);
  }
  return;
}

bool ArgList::isInList(llvm::Value *V) {
  if (find(arg_list.begin(), arg_list.end(), V) != arg_list.end()) {
    return true;
  }
  return false;
}

int64_t ArgList::getOperandNum(llvm::Value *V) {
  auto ele = find(arg_list.begin(), arg_list.end(), V);
  if (ele != arg_list.end()) {
    return distance(arg_list.begin(), ele);
  }
  return ROOT_INDEX;
}

void ArgList::setFreed(llvm::Value *V) {
  this->setFreed(V, std::queue<int>());
  return;
}

void ArgList::setFreed(llvm::Value *V, std::queue<int> indexes) {
  int64_t ind = getOperandNum(V);
  if (ind >= 0 && ind < argNum) stats[ind].setMemberFreed(indexes);
  return;
}

bool ArgList::isFreed(llvm::Value *V, std::queue<int> indexes) {
  int64_t ind = getOperandNum(V);
  if (ind >= 0 && ind < argNum) return stats[ind].isMemberFreed(indexes);
  return false;
}
}  // namespace ST_free
