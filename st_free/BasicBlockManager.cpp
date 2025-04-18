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

void BasicBlockManager::CollectInInfo(BasicBlock *B, bool isEntryPoint) {
  bool isFirst = true;
  if (this->exists(B)) return;

  if (isEntryPoint) this->set(B);

  if (this->checkIfErrorBlock(B)) {
    BBMap[B].setErrorHandlingBlock();
  }

  this->addFreeInfoFromDMZToPreds(B);
  for (BasicBlock *PredBB : predecessors(B)) {
    if (isFirst) {
      this->copy(PredBB, B);
      isFirst = false;
    } else {
      this->unite(PredBB, B);
      this->intersect(PredBB, B);
      this->copyCorrectlyFreed(PredBB, B);
    }
    this->diff(PredBB, B);
  }
  return;
}

void BasicBlockManager::copy(BasicBlock *src, BasicBlock *tgt) {
  BBMap[tgt] = BasicBlockInformation(BBMap[src]);
  BBMap[tgt].setDMZList(this->getBasicBlockRemoveAllocList(src, tgt));
  return;
}

void BasicBlockManager::copyFreed(BasicBlock *src, BasicBlock *tgt) {
  BBMap[tgt].setFreeList(uniteList(this->getBasicBlockFreeList(src),
                                   this->getBasicBlockFreeList(tgt)));
  return;
}

void BasicBlockManager::intersect(BasicBlock *src, BasicBlock *tgt) {
  BBMap[tgt].setFreeList(intersectList(this->getBasicBlockFreeList(src),
                                       this->getBasicBlockFreeList(tgt)));
  // BBMap[tgt].setLiveVariables(intersectLiveVariables(this->getLiveVariables(src),
  // this->getLiveVariables(tgt)));
  return;
}

void BasicBlockManager::unite(BasicBlock *src, BasicBlock *tgt) {
  BBMap[tgt].setAllocList(uniteList(this->getBasicBlockAllocList(tgt),
                                    this->getBasicBlockAllocList(src)));
  BBMap[tgt].setPendingArgAllocList(
      uniteList(this->getBasicBlockPendingAllocList(tgt),
                this->getBasicBlockPendingAllocList(src)));
  BBMap[tgt].setDMZList(
      uniteList(this->getBasicBlockDMZList(tgt),
                this->getBasicBlockRemoveAllocList(src, tgt)));
  return;
}

void BasicBlockManager::diff(BasicBlock *src, BasicBlock *tgt) {
  BBMap[tgt].setAllocList(
      diffList(this->getBasicBlockAllocList(tgt),
               this->get(src)->getRemoveAllocs(tgt).getList()));
  // for (auto ele : this->get(src)->getRemoveAllocs(tgt).getList()) {
  //     if (auto StTy = dyn_cast<StructType>(get_type(ele->getType()))) {
  //         if (StTy->hasName()) {
  //             generateWarning(tgt->getFirstNonPHI(), StTy->getName(), true);
  //             generateWarning(tgt->getFirstNonPHI(),
  //             to_string(ele->getNum()), true);
  //         }
  //     }
  // }
  BBMap[tgt].setPendingArgAllocList(
      diffList(this->getBasicBlockPendingAllocList(tgt),
               this->get(src)->getRemoveAllocs(tgt).getList()));
  return;
}

void BasicBlockManager::addFreeInfoFromDMZToPreds(BasicBlock *src) {
  BasicBlockList freeUnite;
  for (BasicBlock *PredBB : predecessors(src)) {
    freeUnite = uniteList(freeUnite, this->getBasicBlockFreeList(PredBB));
  }

  for (BasicBlock *PredBB : predecessors(src)) {
    BasicBlockInformation *BBInfo = this->get(PredBB);
    if (BBInfo) {
      for (auto ele : freeUnite) {
        if (BBInfo->getDMZList().typeExists(ele->getType())) {
          generateWarning(&(src->front()), "Type Exists");
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

// void BasicBlockManager::copyCorrectlyFreedToPrev(BasicBlock *src){
// for (BasicBlock* PredBB: predecessors(src)) {
//     BasicBlockInformation* BInfo = this->get(PredBB);
//     if(BInfo && BInfo->isLoopBlock())
//         this->copyCorrectlyFreed(src, PredBB);
// }
// }

BasicBlockList BasicBlockManager::intersectList(BasicBlockList src,
                                                BasicBlockList tgt) {
  BasicBlockList tmp;
  llvm::sort(src.begin(), src.end());
  llvm::sort(tgt.begin(), tgt.end());

  set_intersection(src.begin(), src.end(), tgt.begin(), tgt.end(),
                   back_inserter(tmp));
  return tmp;
}

BasicBlockList BasicBlockManager::uniteList(BasicBlockList src,
                                            BasicBlockList tgt) {
  BasicBlockList tmp;
  llvm::sort(src.begin(), src.end());
  llvm::sort(tgt.begin(), tgt.end());

  set_union(src.begin(), src.end(), tgt.begin(), tgt.end(), back_inserter(tmp));
  return tmp;
}

BasicBlockList BasicBlockManager::diffList(BasicBlockList src,
                                           BasicBlockList tgt) {
  BasicBlockList tmp;
  llvm::sort(src.begin(), src.end());
  llvm::sort(tgt.begin(), tgt.end());

  set_difference(src.begin(), src.end(), tgt.begin(), tgt.end(),
                 back_inserter(tmp));
  return tmp;
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
      // this->intersect(src, SucBB);
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
    }
  }

  return tempErrorBlock;
}
}  // namespace ST_free
