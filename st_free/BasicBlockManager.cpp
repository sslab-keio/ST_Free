#include "include/BasicBlockManager.hpp"

namespace ST_free {

BasicBlockWorkList::BasicBlockWorkList() { MarkedValues = BasicBlockList(); }

BasicBlockWorkList::BasicBlockWorkList(const BasicBlockList v) {
  MarkedValues = BasicBlockList(v);
}

void BasicBlockWorkList::add(const UniqueKey *UK) { MarkedValues.insert(UK); }

void BasicBlockWorkList::add(BasicBlockList list) {
  for (auto ele : list) {
    MarkedValues.insert(ele);
  }
}

bool BasicBlockWorkList::exists(const UniqueKey *UK) {
  if (MarkedValues.find(UK) != MarkedValues.end()) return true;
  return false;
}

bool BasicBlockWorkList::typeExists(llvm::Type *T) {
  auto foundVal =
      find_if(MarkedValues.begin(), MarkedValues.end(),
              [T](const UniqueKey *UK) { return UK->getType() == T; });
  if (foundVal != MarkedValues.end()) return true;
  return false;
}

const UniqueKey *BasicBlockWorkList::getFromType(llvm::Type *T) {
  auto foundVal =
      find_if(MarkedValues.begin(), MarkedValues.end(),
              [T](const UniqueKey *UK) { return UK->getType() == T; });
  if (foundVal != MarkedValues.end()) return *foundVal;
  return NULL;
}

BasicBlockList BasicBlockWorkList::getWithParentType(llvm::Type *T) {
  BasicBlockList with_parent_type;
  auto current_pos = MarkedValues.begin();
  while (current_pos != MarkedValues.end()) {
    current_pos =
        find_if(current_pos, MarkedValues.end(),
                [T](const UniqueKey *UK) { return UK->getType() == T; });
    if (current_pos != MarkedValues.end()) {
      with_parent_type.insert(*current_pos);
      current_pos++;
    }
  }
  return with_parent_type;
}

bool BasicBlockWorkList::valueExists(llvm::Value *V) {
  auto foundVal =
      find_if(MarkedValues.begin(), MarkedValues.end(),
              [V](const UniqueKey *UK) { return UK->getValue() == V; });
  if (foundVal != MarkedValues.end()) return true;
  return false;
}

bool BasicBlockWorkList::fieldExists(llvm::Type *T, long ind) {
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

const UniqueKey *BasicBlockWorkList::getUKFromValue(llvm::Value *V) {
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
  loopHeaderBlock = false;
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
  loopHeaderBlock = false;
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

BasicBlockList BasicBlockInformation::getAllocList() const {
  return allocList.getList();
}

BasicBlockList BasicBlockInformation::getFreeList() const {
  return freeList.getList();
}

BasicBlockList BasicBlockInformation::getErrorRemovedAllocList(
    llvm::BasicBlock *tgt, BasicBlockWorkList free_pool) {
  BasicBlockWorkList struct_member_added = getRemoveAllocs(tgt);
  for (auto ele : getRemoveAllocs(tgt).getList()) {
    if (auto stTy =
            llvm::dyn_cast<llvm::StructType>(get_type(ele->getType()))) {
      for (llvm::Type *mem_ty : stTy->elements()) {
        struct_member_added.add(free_pool.getWithParentType(mem_ty));
      }
    }
  }

  BasicBlockList tmp = BasicBlockListOperation::intersectList(
      allocList.getList(), struct_member_added.getList());

  return BasicBlockListOperation::diffList(allocList.getList(), tmp);
}

BasicBlockList BasicBlockInformation::getErrorAddedFreeList(
    llvm::BasicBlock *tgt, BasicBlockWorkList free_pool) {
  BasicBlockWorkList struct_member_added = getRemoveAllocs(tgt);
  for (auto ele : getRemoveAllocs(tgt).getList()) {
    if (auto stTy =
            llvm::dyn_cast<llvm::StructType>(get_type(ele->getType()))) {
      for (llvm::Type *mem_ty : stTy->elements()) {
        struct_member_added.add(free_pool.getWithParentType(mem_ty));
      }
    }
  }

  return BasicBlockListOperation::uniteList(freeList.getList(),
                                            struct_member_added.getList());
}

// BasicBlockList BasicBlockInformation::getShrinkedBaseList() const {
//   return BasicBlockListOperation::intersectList(freeList.getList(),
//                                                 allocList.getList());
// }

// BasicBlockList BasicBlockInformation::getShrinkedAllocList(BasicBlock *tgt) {
//   BasicBlockList tmpList = BasicBlockListOperation::diffList(
//       getShrinkedBaseList(), getRemoveAllocs(tgt).getList());

//   return BasicBlockListOperation::diffList(tmpList, getAllocList());
// }

// BasicBlockList BasicBlockInformation::getShrinkedFreeList(BasicBlock *tgt) {
//   BasicBlockList tmpList = BasicBlockListOperation::diffList(
//       getShrinkedBaseList(), getRemoveAllocs(tgt).getList());

//   return BasicBlockListOperation::diffList(tmpList, getFreeList());
// }

void BasicBlockInformation::addFree(const UniqueKey *UK) { freeList.add(UK); }

void BasicBlockInformation::setCorrectlyBranched() { correctlyBranched = true; }

void BasicBlockInformation::setErrorHandlingBlock() {
  errorHandlingBlock = true;
}

bool BasicBlockInformation::isErrorHandlingBlock() {
  return errorHandlingBlock;
}

void BasicBlockInformation::addSucceedingErrorBlock(llvm::BasicBlock *B) {
  succeedingErrorBlocks.push_back(B);
}

bool BasicBlockInformation::isInSucceedingErrorBlock(llvm::BasicBlock *B) {
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

bool BasicBlockInformation::LiveVariableExists(llvm::Value *v) {
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

void BasicBlockInformation::addLiveVariable(llvm::Value *v) {
  liveVariables.push_back(v);
}

LiveVariableList BasicBlockInformation::getLiveVariables() const {
  return liveVariables;
}
void BasicBlockInformation::setLiveVariables(LiveVariableList lvl) {
  liveVariables = LiveVariableList(lvl);
}

bool BasicBlockManager::exists(llvm::BasicBlock *B) {
  if (BBMap.find(B) != BBMap.end()) return true;
  return false;
}

void BasicBlockManager::CollectInInfo(
    llvm::BasicBlock *B, bool isEntryPoint,
    const std::map<const UniqueKey *, const UniqueKey *> *alias_map) {
  bool isFirst = true;
  if (isEntryPoint) this->set(B);

  BBMap[B].backupFreeAllocInformation();

  if (this->checkIfErrorBlock(B)) {
    generateWarning(B->getFirstNonPHI(), "Is error Block");
    BBMap[B].setErrorHandlingBlock();
  }
  this->addFreeInfoFromDMZToPreds(B);

  // Call this function incase this is the exiting block of a loop. Check if
  // there are any path bypassing the loop, and remove them from predecessors
  // to pass on the correct information
  std::vector<llvm::BasicBlock *> optimized_preds = optimizeLoopPredecessors(B);

  // A pool of freed fields from all the predecessors. This is used later on to
  // determine any members of the struct that are NULL checked, and add them
  // to the free list as well.
  BasicBlockWorkList free_pool;
  for (llvm::BasicBlock *PredBB : optimized_preds) {
    free_pool.setList(BasicBlockListOperation::uniteList(
        free_pool.getList(), this->getBasicBlockFreeList(PredBB)));
  }
  // if (B->hasName())
  //   STFREE_LOG_ON(B->getFirstNonPHI(), B->getName());
  for (llvm::BasicBlock *PredBB : optimized_preds) {
    // if (PredBB->hasName())
    //   generateWarning(PredBB->getFirstNonPHI(), PredBB->getName(), true);
    if (isFirst) {
      this->copyAllList(PredBB, B, free_pool);
      isFirst = false;
    } else {
      this->uniteAllocList(PredBB, B, free_pool);
      this->uniteDMZList(PredBB, B, free_pool);
      this->intersectFreeList(PredBB, B, free_pool);
    }
  }

  return;
}

void BasicBlockManager::copyAllList(llvm::BasicBlock *src,
                                    llvm::BasicBlock *tgt,
                                    BasicBlockWorkList free_pool) {
  BBMap[tgt].setFreeList(
      this->getBasicBlockErrorAddedFreeList(src, tgt, free_pool));
  BBMap[tgt].setAllocList(
      this->getBasicBlockErrorRemovedAllocList(src, tgt, free_pool));
  BBMap[tgt].setPendingArgAllocList(this->getBasicBlockPendingAllocList(src));
  BBMap[tgt].setLiveVariables(this->getLiveVariables(src));
  BBMap[tgt].setDMZList(
      this->getBasicBlockRemoveAllocList(src, tgt, free_pool));
  return;
}

void BasicBlockManager::copyFreed(llvm::BasicBlock *src,
                                  llvm::BasicBlock *tgt) {
  BBMap[tgt].setFreeList(BasicBlockListOperation::uniteList(
      this->getBasicBlockFreeList(src), this->getBasicBlockFreeList(tgt)));
  return;
}

void BasicBlockManager::intersectFreeList(llvm::BasicBlock *src,
                                          llvm::BasicBlock *tgt,
                                          BasicBlockWorkList free_pool) {
  BBMap[tgt].setFreeList(BasicBlockListOperation::intersectList(
      this->getBasicBlockFreeList(tgt),
      this->getBasicBlockErrorAddedFreeList(src, tgt, free_pool)));
  return;
}

void BasicBlockManager::uniteAllocList(llvm::BasicBlock *src,
                                       llvm::BasicBlock *tgt,
                                       BasicBlockWorkList free_pool) {
  BBMap[tgt].setAllocList(BasicBlockListOperation::uniteList(
      this->getBasicBlockAllocList(tgt),
      this->getBasicBlockErrorRemovedAllocList(src, tgt, free_pool)));

  BBMap[tgt].setPendingArgAllocList(BasicBlockListOperation::uniteList(
      this->getBasicBlockPendingAllocList(tgt),
      this->getBasicBlockPendingAllocList(src)));
}

void BasicBlockManager::uniteDMZList(llvm::BasicBlock *src,
                                     llvm::BasicBlock *tgt,
                                     BasicBlockWorkList free_pool) {
  BBMap[tgt].setDMZList(BasicBlockListOperation::uniteList(
      this->getBasicBlockDMZList(tgt),
      this->getBasicBlockRemoveAllocList(src, tgt, free_pool)));
}

void BasicBlockManager::removeAllocatedInError(
    llvm::BasicBlock *src, llvm::BasicBlock *tgt,
    const std::map<const UniqueKey *, const UniqueKey *> *alias_map) {
  BasicBlockWorkList remove_allocs = this->get(src)->getRemoveAllocs(tgt);

  generateWarning(
      tgt->getFirstNonPHI(),
      "Remove Alloc: " + std::to_string(remove_allocs.getList().size()));

  for (auto ele : this->get(src)->getRemoveAllocs(tgt).getList()) {
    auto aliased_value = alias_map->find(ele);
    if (aliased_value != alias_map->end()) {
      remove_allocs.add(aliased_value->second);
    }
  }

  BasicBlockList tmpList = BasicBlockListOperation::intersectList(
      this->getBasicBlockAllocList(tgt), remove_allocs.getList());

  BBMap[tgt].setAllocList(BasicBlockListOperation::diffList(
      this->getBasicBlockAllocList(tgt), tmpList));

  BBMap[tgt].setFreeList(BasicBlockListOperation::uniteList(
      this->getBasicBlockFreeList(tgt), remove_allocs.getList()));

  tmpList = BasicBlockListOperation::intersectList(
      this->getBasicBlockPendingAllocList(tgt), remove_allocs.getList());
  BBMap[tgt].setPendingArgAllocList(BasicBlockListOperation::diffList(
      this->getBasicBlockPendingAllocList(tgt), tmpList));
}

void BasicBlockManager::shrinkFreedFromAlloc(llvm::BasicBlock *B) {
  BasicBlockList removable_list = BasicBlockListOperation::intersectList(
      this->getBasicBlockAllocList(B), this->getBasicBlockFreeList(B));

  BBMap[B].setAllocList(BasicBlockListOperation::diffList(
      this->getBasicBlockAllocList(B), removable_list));
  BBMap[B].setFreeList(BasicBlockListOperation::diffList(
      this->getBasicBlockFreeList(B), removable_list));
}

void BasicBlockManager::addFreeInfoFromDMZToPreds(llvm::BasicBlock *src) {
  BasicBlockList freeUnite;
  for (llvm::BasicBlock *PredBB : llvm::predecessors(src)) {
    freeUnite = BasicBlockListOperation::uniteList(
        freeUnite, this->getBasicBlockFreeList(PredBB));
  }

  for (llvm::BasicBlock *PredBB : llvm::predecessors(src)) {
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

void BasicBlockManager::copyCorrectlyFreed(llvm::BasicBlock *src,
                                           llvm::BasicBlock *tgt) {
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

BasicBlockList BasicBlockManager::getBasicBlockFreeList(llvm::BasicBlock *src) {
  if (this->exists(src)) {
    return BBMap[src].getFreeList();
  }
  return BasicBlockList();
}

BasicBlockList BasicBlockManager::getBasicBlockAllocList(
    llvm::BasicBlock *src) {
  if (this->exists(src)) {
    return BBMap[src].getAllocList();
  }
  return BasicBlockList();
}

BasicBlockList BasicBlockManager::getBasicBlockErrorAddedFreeList(
    llvm::BasicBlock *src, llvm::BasicBlock *tgt,
    BasicBlockWorkList free_pool) {
  if (this->exists(src)) {
    return BBMap[src].getErrorAddedFreeList(tgt, free_pool);
  }
  return BasicBlockList();
}

BasicBlockList BasicBlockManager::getBasicBlockErrorRemovedAllocList(
    llvm::BasicBlock *src, llvm::BasicBlock *tgt,
    BasicBlockWorkList free_pool) {
  if (this->exists(src)) {
    return BBMap[src].getErrorRemovedAllocList(tgt, free_pool);
  }
  return BasicBlockList();
}

BasicBlockList BasicBlockManager::getShrinkedBasicBlockFreeList(
    llvm::BasicBlock *src, llvm::BasicBlock *tgt) {
  BasicBlockList tmpList;
  if (this->exists(src)) {
    tmpList = BBMap[src].getWorkList(FREED).getList();
    return BBMap[src].getWorkList(FREED).getList();
  }
  return tmpList;
}

BasicBlockList BasicBlockManager::getShrinkedBasicBlockAllocList(
    llvm::BasicBlock *src, llvm::BasicBlock *tgt) {
  if (this->exists(src)) {
    return BBMap[src].getWorkList(ALLOCATED).getList();
  }
  return BasicBlockList();
}

BasicBlockList BasicBlockManager::getBasicBlockDMZList(llvm::BasicBlock *src) {
  if (this->exists(src)) return BBMap[src].getDMZList().getList();
  return BasicBlockList();
}

BasicBlockList BasicBlockManager::getBasicBlockPendingAllocList(
    llvm::BasicBlock *src) {
  if (this->exists(src)) return BBMap[src].getPendingArgAllocList().getList();
  return BasicBlockList();
}

BasicBlockList BasicBlockManager::getBasicBlockRemoveAllocList(
    llvm::BasicBlock *src, llvm::BasicBlock *tgt,
    BasicBlockWorkList free_pool) {
  if (this->exists(src)) return BBMap[src].getRemoveAllocs(tgt).getList();
  return BasicBlockList();
}

BasicBlockInformation *BasicBlockManager::get(llvm::BasicBlock *B) {
  if (this->exists(B)) return &BBMap[B];
  return NULL;
}

LiveVariableList BasicBlockManager::getLiveVariables(llvm::BasicBlock *B) {
  return BBMap[B].getLiveVariables();
}

bool BasicBlockManager::isPredBlockCorrectlyBranched(llvm::BasicBlock *B) {
  if (llvm::pred_size(B) == 1) {
    for (llvm::BasicBlock *PredBB : llvm::predecessors(B)) {
      if (BBMap[PredBB].isCorrectlyBranched()) return true;
    }
  }
  return false;
}

void BasicBlockManager::set(llvm::BasicBlock *B) {
  if (!this->exists(B)) BBMap[B] = BasicBlockInformation();
}

void BasicBlockManager::updateSuccessorBlock(llvm::BasicBlock *src) {
  for (llvm::BasicBlock *SucBB : llvm::successors(src)) {
    if (this->exists(SucBB)) {
      this->copyCorrectlyFreed(src, SucBB);
    }
  }
}

void BasicBlockInformation::addStoredCallValues(llvm::Value *v,
                                                llvm::CallInst *CI) {
  storedCallValues.push_back(std::pair<llvm::Value *, llvm::CallInst *>(v, CI));
}

std::vector<std::pair<llvm::Value *, llvm::CallInst *>>
BasicBlockInformation::getStoredCallValues() {
  return storedCallValues;
}

bool BasicBlockInformation::isCallValues(llvm::Value *V) {
  auto fele = find_if(storedCallValues.begin(), storedCallValues.end(),
                      [V](std::pair<llvm::Value *, llvm::CallInst *> &p) {
                        return p.first == V;
                      });
  if (fele != storedCallValues.end()) return true;
  return false;
}

llvm::CallInst *BasicBlockInformation::getCallInstForVal(llvm::Value *V) {
  auto fele = find_if(storedCallValues.begin(), storedCallValues.end(),
                      [V](std::pair<llvm::Value *, llvm::CallInst *> &p) {
                        return p.first == V;
                      });
  if (fele != storedCallValues.end()) return (*fele).second;
  return NULL;
}

void BasicBlockInformation::addRemoveAlloc(llvm::BasicBlock *B, UniqueKey *UK) {
  if (!B || !UK) return;
  if (removeAllocs.find(B) == removeAllocs.end())
    removeAllocs[B] = BasicBlockWorkList();
  removeAllocs[B].add(UK);
  return;
}

BasicBlockWorkList BasicBlockInformation::getRemoveAllocs(llvm::BasicBlock *B) {
  if (removeAllocs.find(B) != removeAllocs.end()) return removeAllocs[B];
  return BasicBlockWorkList();
}

bool BasicBlockManager::checkIfErrorBlock(llvm::BasicBlock *B) {
  bool tempErrorBlock = false;

  for (llvm::BasicBlock *PredBB : llvm::predecessors(B)) {
    if (!this->exists(PredBB)) continue;

    if (this->get(PredBB)->isInSucceedingErrorBlock(B)) tempErrorBlock = true;

    if (this->getBasicBlockRemoveAllocList(PredBB, B).size() == 0 &&
        !this->get(PredBB)->isErrorHandlingBlock()) {
      tempErrorBlock = false;
      break;
    }
  }

  return tempErrorBlock;
}

void BasicBlockInformation::backupFreeAllocInformation() {
  BackupAllocList = BasicBlockWorkList(allocList);
  BackupFreeList = BasicBlockWorkList(freeList);
}

void BasicBlockInformation::clearBackup() {
  BackupAllocList = BasicBlockWorkList();
  BackupFreeList = BasicBlockWorkList();
}

bool BasicBlockInformation::isInformationIdenticalToBackup() {
  if (BackupFreeList.getList().size() != freeList.getList().size() ||
      BackupAllocList.getList().size() != allocList.getList().size())
    return false;

  return BackupFreeList.getList() == freeList.getList() &&
         BackupAllocList.getList() == allocList.getList();
}

std::vector<llvm::BasicBlock *> BasicBlockManager::optimizeLoopPredecessors(
    llvm::BasicBlock *tgt) {
  std::vector<llvm::BasicBlock *> collected_info;
  std::vector<llvm::BasicBlock *> loop_header_predecessors;
  bool isLoopExitingBlock = false;

  if (!BBMap[tgt].isLoopBlock()) {
    for (llvm::BasicBlock *PredBB : llvm::predecessors(tgt)) {
      if (exists(PredBB) && BBMap[PredBB].isLoopBlock()) {
        generateWarning(tgt->getFirstNonPHI(), "[LOOP] Is Loop Exiting Block");
        isLoopExitingBlock = true;

        // 1. Look at the header block
        llvm::BasicBlock *topBlock = BBMap[PredBB].getLoop()->getHeader();

        // 2. If the loop contains a preheader block, goto the block before the
        //   predecessor and check
        if (llvm::BasicBlock *preheader =
                BBMap[PredBB].getLoop()->getLoopPreheader()) {
          generateWarning(PredBB->getFirstNonPHI(), "[LOOP] Is preheader block", true);
          topBlock = preheader;
        }

        for (llvm::BasicBlock *loop_header_preds : llvm::predecessors(topBlock))
          loop_header_predecessors.push_back(loop_header_preds);
      }
    }
  }

  for (llvm::BasicBlock *PredBB : llvm::predecessors(tgt)) {
    if (isLoopExitingBlock && exists(PredBB) && !BBMap[PredBB].isLoopBlock()) {
      generateWarning(tgt->getFirstNonPHI(), "[LOOP] Has by passing path");
      
      // 2. If the predecessor of the header contains |PredBB|, then consider it
      //   as before-loop
      if (std::find(loop_header_predecessors.begin(),
                    loop_header_predecessors.end(),
                    PredBB) != loop_header_predecessors.end()) {
        generateWarning(tgt->getFirstNonPHI(), "[LOOP] PredBlock by passes!",
                        true);
        continue;
      }
    }
    collected_info.push_back(PredBB);
  }
  return collected_info;
}

void BasicBlockInformation::setLoopBlock(llvm::Loop *L) {
  loopBlock = true;
  loop = L;
}

const llvm::Loop *BasicBlockInformation::getLoop() { return loop; }

BasicBlockList BasicBlockListOperation::intersectList(BasicBlockList src,
                                                      BasicBlockList tgt) {
  BasicBlockList tmp;

  set_intersection(src.begin(), src.end(), tgt.begin(), tgt.end(),
                   inserter(tmp, tmp.end()));
  return tmp;
}

BasicBlockList BasicBlockListOperation::uniteList(BasicBlockList src,
                                                  BasicBlockList tgt) {
  BasicBlockList tmp;

  set_union(src.begin(), src.end(), tgt.begin(), tgt.end(),
            inserter(tmp, tmp.end()));
  return tmp;
}

BasicBlockList BasicBlockListOperation::diffList(BasicBlockList src,
                                                 BasicBlockList tgt) {
  BasicBlockList tmp;

  set_difference(src.begin(), src.end(), tgt.begin(), tgt.end(),
                 inserter(tmp, tmp.end()));
  return tmp;
}
}  // namespace ST_free
