#pragma once
#include "ST_free.hpp"
#include "UniqueKeyManager.hpp"
#include "ValueInformation.hpp"

namespace ST_free {
using BasicBlockList = std::set<const UniqueKey *>;
class BasicBlockWorkList {
 public:
  BasicBlockWorkList();
  BasicBlockWorkList(const BasicBlockList);
  void add(const UniqueKey *UK);
  bool exists(const UniqueKey *UK);
  bool typeExists(llvm::Type *T);
  const UniqueKey *getFromType(llvm::Type *T);
  bool valueExists(llvm::Value *V);
  bool fieldExists(llvm::Type *T, long ind);
  const UniqueKey *getUKFromValue(llvm::Value *V);
  BasicBlockList getList() const;
  void setList(BasicBlockList);

 private:
  BasicBlockList MarkedValues;
};

using LiveVariableList = std::vector<llvm::Value *>;
class BasicBlockInformation {
 public:
  enum BasicBlockInformationStat {
    BASIC_BLOCK_STAT_UNANALYZED,
    BASIC_BLOCK_STAT_ANALYZING,
    BASIC_BLOCK_STAT_ANALYZED
  };
  BasicBlockInformation();
  BasicBlockInformation(const BasicBlockInformation &);
  void initLists(const BasicBlockInformation &);

  /*** Free Related Methods ***/
  void addFree(const UniqueKey *UK);
  bool FreeExists(const UniqueKey *UK);
  void setFreeList(BasicBlockList);

  /*** Alloc Related Methods ***/
  void addAlloc(const UniqueKey *UK);
  bool AllocExists(const UniqueKey *UK);
  void setAllocList(BasicBlockList);

  /*** DMZJRelated Methods ***/
  void setDMZList(BasicBlockList);
  BasicBlockWorkList getDMZList() const;

  /*** Pending Alloc Related Methods ***/
  void addPendingArgAlloc(const UniqueKey *UK);
  BasicBlockWorkList getPendingArgAllocList() const;
  void setPendingArgAllocList(BasicBlockList);

  /*** Live Variable Methods ***/
  void setLiveVariables(LiveVariableList);
  void addLiveVariable(llvm::Value *v);
  bool LiveVariableExists(llvm::Value *v);
  void incrementRefCount(llvm::Value *v);
  void decrementRefCount(llvm::Value *v);

  /*** Branch Related Methods ***/
  void setCorrectlyBranched();
  bool isCorrectlyBranched();
  void setErrorHandlingBlock();
  bool isErrorHandlingBlock();
  void addSucceedingErrorBlock(llvm::BasicBlock *B);
  bool isInSucceedingErrorBlock(llvm::BasicBlock *B);

  /*** CorrectlyFreed ***/
  void addCorrectlyFreedValue(const UniqueKey *UK);
  bool CorrectlyFreedValueExists(const UniqueKey *UK);
  BasicBlockWorkList getCorrectlyFreedValues() const;

  /*** Worklist algorithm related ***/
  void backupFreeAllocInformation();
  void clearBackup();
  bool isInformationIdenticalToBackup();

  /*** Utilities ***/
  BasicBlockWorkList getWorkList(int mode) const;
  LiveVariableList getLiveVariables() const;
  void addStoredCallValues(llvm::Value *v, llvm::CallInst *CI);
  std::vector<std::pair<llvm::Value *, llvm::CallInst *>> getStoredCallValues();
  bool isCallValues(llvm::Value *V);
  llvm::CallInst *getCallInstForVal(llvm::Value *V);
  void addRemoveAlloc(llvm::BasicBlock *B, UniqueKey *UK);
  BasicBlockWorkList getRemoveAllocs(llvm::BasicBlock *B);
  void setUnconditionalBranched() { unConditionalBranched = true; }
  bool isUnconditionalBranched() { return unConditionalBranched; }

  /*** Original Lists ***/
  BasicBlockList getAllocList() const;
  BasicBlockList getFreeList() const;

  BasicBlockList getErrorRemovedAllocList(llvm::BasicBlock *tgt);
  BasicBlockList getErrorAddedFreeList(llvm::BasicBlock *tgt);

  /*** Shrinked Lists ***/
  // Returns diff of alloc and free
  // BasicBlockList getShrinkedBaseList() const;
  // BasicBlockList getShrinkedAllocList(BasicBlock *tgt);
  // BasicBlockList getShrinkedFreeList(BasicBlock *tgt);

  /*** Reverse Propagation ***/
  bool isReversePropagated(){return reversepropagated;}
  void setReversePropagated(){reversepropagated = true;}
  /*** Status Information ***/
  BasicBlockInformationStat getStatus() { return information_status; };
  void setStatusToAnalyzing() {
    information_status = BasicBlockInformationStat::BASIC_BLOCK_STAT_ANALYZING;
  }
  void setStatusToAnalyzed() {
    information_status = BasicBlockInformationStat::BASIC_BLOCK_STAT_ANALYZED;
  }
  void setStatusToUnanalyzed() {
    information_status = BasicBlockInformationStat::BASIC_BLOCK_STAT_UNANALYZED;
  }

 private:
  /*** BasicBlock Lists ***/
  BasicBlockWorkList freeList;
  BasicBlockWorkList allocList;
  BasicBlockWorkList pendingArgStoreList;
  BasicBlockWorkList correctlyFreed;
  BasicBlockWorkList dmzList;
  LiveVariableList liveVariables;

  /*** Backup Lists to conduct worklist algorithm ***/
  BasicBlockWorkList BackupFreeList;
  BasicBlockWorkList BackupAllocList;

  std::vector<std::pair<llvm::Value *, llvm::CallInst *>> storedCallValues;
  std::map<llvm::BasicBlock *, BasicBlockWorkList> removeAllocs;
  std::vector<llvm::BasicBlock *> succeedingErrorBlocks;

  /*** BasicBlock Status ***/
  bool correctlyBranched;
  bool predCorrectlyBranched;
  bool loopBlock;
  bool errorHandlingBlock;
  bool unConditionalBranched;
  bool reversepropagated;

  /*** BasicBlockInformation Status ***/
  BasicBlockInformationStat information_status;
};

class BasicBlockManager {
 public:
  /*** getter ***/
  void set(llvm::BasicBlock *B);
  BasicBlockInformation *get(llvm::BasicBlock *B);
  BasicBlockList getBasicBlockAllocList(llvm::BasicBlock *src);
  BasicBlockList getBasicBlockFreeList(llvm::BasicBlock *src);
  BasicBlockList getBasicBlockPendingAllocList(llvm::BasicBlock *src);

  BasicBlockList getBasicBlockRemoveAllocList(llvm::BasicBlock *src, llvm::BasicBlock *tgt);
  BasicBlockList getBasicBlockErrorRemovedAllocList(llvm::BasicBlock *src, llvm::BasicBlock *tgt);
  BasicBlockList getBasicBlockErrorAddedFreeList(llvm::BasicBlock *src, llvm::BasicBlock *tgt);

  BasicBlockList getBasicBlockDMZList(llvm::BasicBlock *src);
  LiveVariableList getLiveVariables(llvm::BasicBlock *B);

  BasicBlockList getShrinkedBasicBlockFreeList(llvm::BasicBlock *src, llvm::BasicBlock *tgt);
  BasicBlockList getShrinkedBasicBlockAllocList(llvm::BasicBlock *src, llvm::BasicBlock *tgt);

  /*** Mediator ***/
  void CollectInInfo(
     llvm:: BasicBlock *B, bool isEntryPoint,
      const std::map<const UniqueKey *, const UniqueKey *> *alias_map);
  void copyAllList(llvm::BasicBlock *src, llvm::BasicBlock *tgt);
  void copyFreed(llvm::BasicBlock *src, llvm::BasicBlock *tgt);
  void copyCorrectlyFreed(llvm::BasicBlock *src, llvm::BasicBlock *tgt);
  void uniteAllocList(llvm::BasicBlock *src, llvm::BasicBlock *tgt);
  void uniteDMZList(llvm::BasicBlock *src, llvm::BasicBlock *tgt);
  void intersectFreeList(llvm::BasicBlock *src, llvm::BasicBlock *tgt);
  void removeAllocatedInError(
      llvm::BasicBlock *src, llvm::BasicBlock *tgt,
      const std::map<const UniqueKey *, const UniqueKey *> *alias_map);
  void updateSuccessorBlock(llvm::BasicBlock *src);
  void addFreeInfoFromDMZToPreds(llvm::BasicBlock *src);
  bool isPredBlockCorrectlyBranched(llvm::BasicBlock *B);
  bool checkIfErrorBlock(llvm::BasicBlock *B);

  /*** Optimizer ***/
  void shrinkFreedFromAlloc(llvm::BasicBlock *B);

 private:
  std::map<llvm::BasicBlock *, BasicBlockInformation> BBMap;
  LiveVariableList intersectLiveVariables(LiveVariableList src,
                                          LiveVariableList tgt);
  bool exists(llvm::BasicBlock *B);
};

namespace BasicBlockListOperation {
BasicBlockList intersectList(BasicBlockList src, BasicBlockList tgt);
BasicBlockList uniteList(BasicBlockList src, BasicBlockList tgt);
BasicBlockList diffList(BasicBlockList src, BasicBlockList tgt);
};  // namespace BasicBlockListOperation

}  // namespace ST_free
