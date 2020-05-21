#include "include/BasicBlockManager.hpp"

namespace ST_free {

BasicBlockWorkList::BasicBlockWorkList() { MarkedValues = BasicBlockList(); }

BasicBlockWorkList::BasicBlockWorkList(const BasicBlockList v) {
  MarkedValues = BasicBlockList(v);
}

// void BasicBlockWorkList::add(Value *v, Type * ty, long mem){
//     UniqueKey uk(v, ty, mem);
//     MarkedValues.push_back(uk);
// }
void BasicBlockWorkList::add(const UniqueKey *UK) {
  MarkedValues.push_back(UK);
}
// bool BasicBlockWorkList::exists(Value * v, Type * ty, long mem){
//     if(find(MarkedValues.begin(), MarkedValues.end(), UniqueKey(v, ty, mem))
//     != MarkedValues.end())
//         return true;
//     return false;
// }

bool BasicBlockWorkList::exists(const UniqueKey *UK) {
  if (find(MarkedValues.begin(), MarkedValues.end(), UK) != MarkedValues.end())
    return true;
  return false;
}

bool BasicBlockWorkList::typeExists(Type *T) {
  auto foundVal =
      find_if(MarkedValues.begin(), MarkedValues.end(),
              [T](const UniqueKey *UK) { return UK->getType() == T; });
  if (foundVal != MarkedValues.end()) return true;
  return false;
}

const UniqueKey *BasicBlockWorkList::getFromType(Type *T) {
  auto foundVal =
      find_if(MarkedValues.begin(), MarkedValues.end(),
              [T](const UniqueKey *UK) { return UK->getType() == T; });
  if (foundVal != MarkedValues.end()) return *foundVal;
  return NULL;
}

bool BasicBlockWorkList::valueExists(Value *V) {
  auto foundVal =
      find_if(MarkedValues.begin(), MarkedValues.end(),
              [V](const UniqueKey *UK) { return UK->getValue() == V; });
  if (foundVal != MarkedValues.end()) return true;
  return false;
}

bool BasicBlockWorkList::fieldExists(Type *T, long ind) {
  auto foundVal = find_if(MarkedValues.begin(), MarkedValues.end(),
                          [T, ind](const UniqueKey *UK) {
                            if (UK->getType() == T) {
                              return UK->getNum() == ind;
                            }
                            return false;
                          });
  if (foundVal != MarkedValues.end()) return true;
  return false;
}

const UniqueKey *BasicBlockWorkList::getUKFromValue(Value *V) {
  auto foundVal =
      find_if(MarkedValues.begin(), MarkedValues.end(),
              [V](const UniqueKey *UK) { return UK->getValue() == V; });
  if (foundVal != MarkedValues.end()) return (*foundVal);
  return NULL;
}

void BasicBlockWorkList::setList(BasicBlockList v) {
  MarkedValues = BasicBlockList(v);
}

BasicBlockInformation::BasicBlockInformation() {
  correctlyBranched = false;
  loopBlock = false;
  errorHandlingBlock = false;
  unConditionalBranched = false;
  reversepropagated = false;
  information_status = BasicBlockInformationStat::BASIC_BLOCK_STAT_UNANALYZED;
}

BasicBlockInformation::BasicBlockInformation(
    const BasicBlockInformation &BStat) {
  freeList = BasicBlockWorkList(BStat.getWorkList(FREED).getList());
  allocList = BasicBlockWorkList(BStat.getWorkList(ALLOCATED).getList());
  pendingArgStoreList = BasicBlockWorkList(BStat.getPendingArgAllocList());
  liveVariables = LiveVariableList(BStat.getLiveVariables());
  correctlyFreed = BasicBlockWorkList(BStat.getCorrectlyFreedValues());
  correctlyBranched = false;
  loopBlock = false;
  errorHandlingBlock = false;
  unConditionalBranched = false;
  reversepropagated = false;
  information_status = BasicBlockInformationStat::BASIC_BLOCK_STAT_UNANALYZED;
}

void BasicBlockInformation::initLists(const BasicBlockInformation &BStat) {
  freeList = BasicBlockWorkList(BStat.getWorkList(FREED).getList());
  allocList = BasicBlockWorkList(BStat.getWorkList(ALLOCATED).getList());
  pendingArgStoreList = BasicBlockWorkList(BStat.getPendingArgAllocList());
  liveVariables = LiveVariableList(BStat.getLiveVariables());
  correctlyFreed = BasicBlockWorkList(BStat.getCorrectlyFreedValues());
}

BasicBlockWorkList BasicBlockInformation::getWorkList(int mode) const {
  if (mode == FREED) return freeList;
  return allocList;
}

BasicBlockWorkList BasicBlockInformation::getDMZList() const { return dmzList; }

BasicBlockWorkList BasicBlockInformation::getPendingArgAllocList() const {
  return pendingArgStoreList;
}

BasicBlockList BasicBlockWorkList::getList() const { return MarkedValues; }

// void BasicBlockInformation::addFree(Value *v, Type * ty , long mem){
//     freeList.add(v, ty, mem);
// }

void BasicBlockInformation::addFree(const UniqueKey *UK) { freeList.add(UK); }

void BasicBlockInformation::setCorrectlyBranched() { correctlyBranched = true; }

void BasicBlockInformation::setLoopBlock() { loopBlock = true; }

bool BasicBlockInformation::isLoopBlock() { return loopBlock; }

void BasicBlockInformation::setErrorHandlingBlock() {
  errorHandlingBlock = true;
}

bool BasicBlockInformation::isErrorHandlingBlock() {
  return errorHandlingBlock;
}

void BasicBlockInformation::addSucceedingErrorBlock(BasicBlock *B) {
  succeedingErrorBlocks.push_back(B);
}

bool BasicBlockInformation::isInSucceedingErrorBlock(BasicBlock *B) {
  if (find(succeedingErrorBlocks.begin(), succeedingErrorBlocks.end(), B) !=
      succeedingErrorBlocks.end())
    return true;
  return false;
}

bool BasicBlockInformation::isCorrectlyBranched() { return correctlyBranched; }

// void BasicBlockInformation::addCorrectlyFreedValue(Value * V, Type * T, long
// mem){
//     correctlyFreed.add(V, T, mem);
// }

void BasicBlockInformation::addCorrectlyFreedValue(const UniqueKey *UK) {
  correctlyFreed.add(UK);
}

// bool BasicBlockInformation::CorrectlyFreedValueExists(Value * V, Type * T,
// long mem){
//     return correctlyFreed.exists(V, T, mem);
// }

bool BasicBlockInformation::CorrectlyFreedValueExists(const UniqueKey *UK) {
  return correctlyFreed.exists(UK);
}

BasicBlockWorkList BasicBlockInformation::getCorrectlyFreedValues() const {
  return correctlyFreed;
}

// bool BasicBlockInformation::FreeExists(Value *v, Type * ty, long mem){
//     return freeList.exists(v, ty, mem);
// }

bool BasicBlockInformation::FreeExists(const UniqueKey *UK) {
  return freeList.exists(UK);
}

void BasicBlockInformation::setFreeList(BasicBlockList v) {
  freeList.setList(v);
}

// void BasicBlockInformation::addAlloc(Value *v, Type * ty, long mem){
//     allocList.add(v, ty, mem);
// }

void BasicBlockInformation::addAlloc(const UniqueKey *UK) { allocList.add(UK); }

bool BasicBlockInformation::LiveVariableExists(Value *v) {
  if (find(liveVariables.begin(), liveVariables.end(), v) !=
      liveVariables.end())
    return true;
  return false;
}

// bool BasicBlockInformation::AllocExists(Value *v, Type *ty, long mem){
//     return allocList.exists(v, ty, mem);
// }

bool BasicBlockInformation::AllocExists(const UniqueKey *UK) {
  return allocList.exists(UK);
}

void BasicBlockInformation::setAllocList(BasicBlockList v) {
  allocList.setList(v);
}

void BasicBlockInformation::setDMZList(BasicBlockList v) { dmzList.setList(v); }

void BasicBlockInformation::addPendingArgAlloc(const UniqueKey *UK) {
  pendingArgStoreList.add(UK);
}

void BasicBlockInformation::setPendingArgAllocList(BasicBlockList v) {
  pendingArgStoreList.setList(v);
}

void BasicBlockInformation::addLiveVariable(Value *v) {
  liveVariables.push_back(v);
}

LiveVariableList BasicBlockInformation::getLiveVariables() const {
  return liveVariables;
}
void BasicBlockInformation::setLiveVariables(LiveVariableList lvl) {
  liveVariables = LiveVariableList(lvl);
}

bool BasicBlockManager::exists(BasicBlock *B) {
  if (BBMap.find(B) != BBMap.end()) return true;
  return false;
}

void BasicBlockManager::CollectInInfo(
    BasicBlock *B, bool isEntryPoint,
    const map<const UniqueKey *, const UniqueKey *> *alias_map) {
  bool isFirst = true;
  if (this->exists(B)) return;

  if (isEntryPoint) this->set(B);

  if (this->checkIfErrorBlock(B)) {
    generateWarning(B->getFirstNonPHI(), "Is error Block", true);
    BBMap[B].setErrorHandlingBlock();
  }

  this->addFreeInfoFromDMZToPreds(B);
  for (BasicBlock *PredBB : predecessors(B)) {
    if (isFirst) {
      this->copyAllList(PredBB, B);
      isFirst = false;
    } else {
      this->uniteAllocList(PredBB, B);
      this->uniteDMZList(PredBB, B);
      this->intersectFreeList(PredBB, B);
      this->copyCorrectlyFreed(PredBB, B);
    }
    this->removeAllocatedInError(PredBB, B, alias_map);
  }

  if (get(B)->isErrorHandlingBlock()) {
    for (BasicBlock *PredBB : predecessors(B)) {
      if (this->exists(PredBB) && get(PredBB)->isErrorHandlingBlock() &&
          get(PredBB)->isUnconditionalBranched()) {
        generateWarning(PredBB->getFirstNonPHI(), "Error and unconditional");
        BBMap[B].setFreeList(BasicBlockListOperation::uniteList(
            this->getBasicBlockFreeList(B),
            this->getBasicBlockFreeList(PredBB)));
      }
    }
  }
  return;
}

void BasicBlockManager::copyAllList(BasicBlock *src, BasicBlock *tgt) {
  BBMap[tgt].initLists(BBMap[src]);
  BBMap[tgt].setDMZList(this->getBasicBlockRemoveAllocList(src, tgt));
  return;
}

void BasicBlockManager::copyFreed(BasicBlock *src, BasicBlock *tgt) {
  BBMap[tgt].setFreeList(BasicBlockListOperation::uniteList(
      this->getBasicBlockFreeList(src), this->getBasicBlockFreeList(tgt)));
  return;
}

void BasicBlockManager::intersectFreeList(BasicBlock *src, BasicBlock *tgt) {
  BBMap[tgt].setFreeList(BasicBlockListOperation::intersectList(
      this->getBasicBlockFreeList(src), this->getBasicBlockFreeList(tgt)));
  return;
}

void BasicBlockManager::uniteAllocList(BasicBlock *src, BasicBlock *tgt) {
  BBMap[tgt].setAllocList(BasicBlockListOperation::uniteList(
      this->getBasicBlockAllocList(tgt), this->getBasicBlockAllocList(src)));
  BBMap[tgt].setPendingArgAllocList(BasicBlockListOperation::uniteList(
      this->getBasicBlockPendingAllocList(tgt),
      this->getBasicBlockPendingAllocList(src)));
}

void BasicBlockManager::uniteDMZList(BasicBlock *src, BasicBlock *tgt) {
  BBMap[tgt].setDMZList(BasicBlockListOperation::uniteList(
      this->getBasicBlockDMZList(tgt),
      this->getBasicBlockRemoveAllocList(src, tgt)));
}

void BasicBlockManager::removeAllocatedInError(
    BasicBlock *src, BasicBlock *tgt,
    const map<const UniqueKey *, const UniqueKey *> *alias_map) {
  BasicBlockWorkList remove_allocs = this->get(src)->getRemoveAllocs(tgt);

  generateWarning(tgt->getFirstNonPHI(), "Remove Alloc: " +
  to_string(remove_allocs.getList().size()), true);

  for (auto ele : this->get(src)->getRemoveAllocs(tgt).getList()) {
    auto aliased_value = alias_map->find(ele);
    if (aliased_value != alias_map->end()) {
      remove_allocs.add(aliased_value->second);
    }
  }

  generateWarning(tgt->getFirstNonPHI(), "After Remove Alloc: " +
  to_string(remove_allocs.getList().size()), true);

  BBMap[tgt].setAllocList(BasicBlockListOperation::diffList(
      this->getBasicBlockAllocList(tgt), remove_allocs.getList()));
  // generateWarning(tgt->getFirstNonPHI(), "After Alloc: " +
  // to_string(getBasicBlockAllocList(tgt).size()), true);
  // generateWarning(tgt->getFirstNonPHI(), "Free: " +
  // to_string(getBasicBlockFreeList(tgt).size()), true);
  BBMap[tgt].setFreeList(BasicBlockListOperation::uniteList(
      this->getBasicBlockFreeList(tgt),
      remove_allocs.getList()));
  // generateWarning(tgt->getFirstNonPHI(), "After Free: " +
  // to_string(getBasicBlockFreeList(tgt).size()), true);
  BBMap[tgt].setPendingArgAllocList(BasicBlockListOperation::diffList(
      this->getBasicBlockPendingAllocList(tgt),
      remove_allocs.getList()));
}

void BasicBlockManager::addFreeInfoFromDMZToPreds(BasicBlock *src) {
  BasicBlockList freeUnite;
  for (BasicBlock *PredBB : predecessors(src)) {
    freeUnite = BasicBlockListOperation::uniteList(
        freeUnite, this->getBasicBlockFreeList(PredBB));
  }

  for (BasicBlock *PredBB : predecessors(src)) {
    BasicBlockInformation *BBInfo = this->get(PredBB);
    if (BBInfo) {
      for (auto ele : freeUnite) {
        if (BBInfo->getDMZList().typeExists(get_type(ele->getType()))) {
          BBInfo->addFree(ele);
        }
      }
    }
  }
  return;
}

void BasicBlockManager::copyCorrectlyFreed(BasicBlock *src, BasicBlock *tgt) {
  for (const UniqueKey *uk : BBMap[src].getCorrectlyFreedValues().getList()) {
    BBMap[tgt].addCorrectlyFreedValue(uk);
  }
}

LiveVariableList BasicBlockManager::intersectLiveVariables(
    LiveVariableList src, LiveVariableList tgt) {
  LiveVariableList tmp;
  llvm::sort(src.begin(), src.end());
  llvm::sort(tgt.begin(), tgt.end());

  set_intersection(src.begin(), src.end(), tgt.begin(), tgt.end(),
                   back_inserter(tmp));

  return tmp;
}

BasicBlockList BasicBlockManager::getBasicBlockFreeList(BasicBlock *src) {
  if (this->exists(src)) {
    return BBMap[src].getWorkList(FREED).getList();
  }
  return BasicBlockList();
}

BasicBlockList BasicBlockManager::getBasicBlockAllocList(BasicBlock *src) {
  if (this->exists(src)) {
    return BBMap[src].getWorkList(ALLOCATED).getList();
  }
  return BasicBlockList();
}

BasicBlockList BasicBlockManager::getBasicBlockDMZList(BasicBlock *src) {
  if (this->exists(src)) return BBMap[src].getDMZList().getList();
  return BasicBlockList();
}

BasicBlockList BasicBlockManager::getBasicBlockPendingAllocList(
    BasicBlock *src) {
  if (this->exists(src)) return BBMap[src].getPendingArgAllocList().getList();
  return BasicBlockList();
}

BasicBlockList BasicBlockManager::getBasicBlockRemoveAllocList(
    BasicBlock *src, BasicBlock *tgt) {
  if (this->exists(src)) return BBMap[src].getRemoveAllocs(tgt).getList();
  return BasicBlockList();
}

BasicBlockInformation *BasicBlockManager::get(BasicBlock *B) {
  if (this->exists(B)) return &BBMap[B];
  return NULL;
}

LiveVariableList BasicBlockManager::getLiveVariables(BasicBlock *B) {
  return BBMap[B].getLiveVariables();
}

bool BasicBlockManager::isPredBlockCorrectlyBranched(BasicBlock *B) {
  if (pred_size(B) == 1) {
    for (BasicBlock *PredBB : predecessors(B)) {
      if (BBMap[PredBB].isCorrectlyBranched()) return true;
    }
  }
  return false;
}

void BasicBlockManager::set(BasicBlock *B) {
  if (!this->exists(B)) BBMap[B] = BasicBlockInformation();
}

void BasicBlockManager::updateSuccessorBlock(BasicBlock *src) {
  for (BasicBlock *SucBB : successors(src)) {
    if (this->exists(SucBB)) {
      this->copyCorrectlyFreed(src, SucBB);
    }
  }
}

void BasicBlockInformation::addStoredCallValues(Value *v, CallInst *CI) {
  storedCallValues.push_back(pair<Value *, CallInst *>(v, CI));
}

vector<pair<Value *, CallInst *>> BasicBlockInformation::getStoredCallValues() {
  return storedCallValues;
}

bool BasicBlockInformation::isCallValues(Value *V) {
  auto fele =
      find_if(storedCallValues.begin(), storedCallValues.end(),
              [V](pair<Value *, CallInst *> &p) { return p.first == V; });
  if (fele != storedCallValues.end()) return true;
  return false;
}

CallInst *BasicBlockInformation::getCallInstForVal(Value *V) {
  auto fele =
      find_if(storedCallValues.begin(), storedCallValues.end(),
              [V](pair<Value *, CallInst *> &p) { return p.first == V; });
  if (fele != storedCallValues.end()) return (*fele).second;
  return NULL;
}
void BasicBlockInformation::addRemoveAlloc(BasicBlock *B, UniqueKey *UK) {
  if (!B || !UK) return;
  if (removeAllocs.find(B) == removeAllocs.end())
    removeAllocs[B] = BasicBlockWorkList();
  removeAllocs[B].add(UK);
  return;
}

BasicBlockWorkList BasicBlockInformation::getRemoveAllocs(BasicBlock *B) {
  if (removeAllocs.find(B) != removeAllocs.end()) return removeAllocs[B];
  return BasicBlockWorkList();
}

bool BasicBlockManager::checkIfErrorBlock(BasicBlock *B) {
  bool tempErrorBlock = true;

  if (!(B->hasNPredecessorsOrMore(1))) tempErrorBlock = false;

  for (BasicBlock *PredBB : predecessors(B)) {
    if (this->exists(PredBB)) {
      if (this->get(PredBB)->isInSucceedingErrorBlock(B)) {
        tempErrorBlock = true;
        break;
      }
      if (this->getBasicBlockRemoveAllocList(PredBB, B).size() == 0) {
        if (!this->get(PredBB)->isErrorHandlingBlock()) tempErrorBlock = false;
      }
    } else {
      tempErrorBlock = false;
    }
  }

  return tempErrorBlock;
}

BasicBlockList BasicBlockListOperation::intersectList(BasicBlockList src,
                                                      BasicBlockList tgt) {
  BasicBlockList tmp;
  llvm::sort(src.begin(), src.end());
  llvm::sort(tgt.begin(), tgt.end());

  set_intersection(src.begin(), src.end(), tgt.begin(), tgt.end(),
                   back_inserter(tmp));
  return tmp;
}

BasicBlockList BasicBlockListOperation::uniteList(BasicBlockList src,
                                                  BasicBlockList tgt) {
  BasicBlockList tmp;
  llvm::sort(src.begin(), src.end());
  llvm::sort(tgt.begin(), tgt.end());

  set_union(src.begin(), src.end(), tgt.begin(), tgt.end(), back_inserter(tmp));
  return tmp;
}

BasicBlockList BasicBlockListOperation::diffList(BasicBlockList src,
                                                 BasicBlockList tgt) {
  BasicBlockList tmp;
  llvm::sort(src.begin(), src.end());
  llvm::sort(tgt.begin(), tgt.end());

  set_difference(src.begin(), src.end(), tgt.begin(), tgt.end(),
                 back_inserter(tmp));
  return tmp;
}
}  // namespace ST_free
