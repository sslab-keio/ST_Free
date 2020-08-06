#pragma once
#include "ST_free.hpp"
#include "support_funcs.hpp"

namespace ST_free {
/**
 * [Class] ArgStatus
 * Keeps Track of each function arguments, and its stats.
 * If the Argument is StructType, and modifies the member of that argument,
 * it add a new ArgStatus Instance to structStatus vector.
 */
class ArgStatus {
 public:
  ArgStatus(uint64_t structSize) {
    freed = false;
    structStatus = std::vector<ArgStatus>(structSize);
    statusSize = structSize;
    T = NULL;
  }
  ArgStatus(llvm::Type *Ty) {
    freed = false;
    T = Ty;
    structStatus = std::vector<ArgStatus>();
    statusSize = 0;
  }
  ArgStatus() {
    freed = false;
    T = NULL;
    structStatus = std::vector<ArgStatus>();
    statusSize = 0;
  }
  bool isFreed() { return freed; }
  void setFreed() { freed = true; }
  void setType(llvm::Type *Ty) { T = Ty; }
  llvm::Type *getType() { return T; }
  bool isStruct() { return T && get_type(T)->isStructTy(); }
  uint size() { return statusSize; }
  int maxSize();
  bool isMemberFreed(std::queue<int> indexes);
  void setMemberFreed(std::queue<int> indexes);
  void extendStatusSize(int extSize);
  ArgStatus *getStatus(int index) {
    return index >= 0 && index < this->size() ? &structStatus[index] : NULL;
  }
  std::vector<bool> getFreedList();

 private:
  bool freed;
  std::vector<ArgStatus> structStatus;
  llvm::Type *T;
  int statusSize;
};

class ArgList {
 public:
  ArgList() { arg_list = std::vector<llvm::Value *>(); }
  explicit ArgList(uint64_t arg_num) {
    argNum = arg_num;
    arg_list = std::vector<llvm::Value *>(arg_num, NULL);
    stats = std::vector<ArgStatus>(arg_num);
  }
  void setArg(uint64_t arg_no, llvm::Value *V);
  llvm::Value *getArg(uint64_t arg_no);
  ArgStatus *getArgStatus(int arg) {
    return (arg >= 0 && arg < stats.size()) ? &stats[arg] : NULL;
  };
  void setArgs(llvm::Function *F);
  bool isInList(llvm::Value *V);
  int64_t getOperandNum(llvm::Value *V);
  void setFreed(llvm::Value *V);
  void setFreed(llvm::Value *V, std::queue<int> indexes);
  bool isFreed(llvm::Value *V, std::queue<int> indexes);

 private:
  uint64_t argNum;
  std::vector<llvm::Value *> arg_list;
  std::vector<ArgStatus> stats;
};
}  // namespace ST_free
