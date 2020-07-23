#pragma once
#include "ST_free.hpp"
#include "UniqueKeyManager.hpp"
#include "ValueInformation.hpp"

namespace ST_free {
using BasicBlockList = set<const UniqueKey *>;
class BasicBlockWorkList {
 public:
  BasicBlockWorkList();
  BasicBlockWorkList(const BasicBlockList);
  void add(const UniqueKey *UK);
  bool exists(const UniqueKey *UK);
  bool typeExists(Type *T);
  const UniqueKey *getFromType(Type *T);
  bool valueExists(Value *V);
  bool fieldExists(Type *T, long ind);
  const UniqueKey *getUKFromValue(Value *V);
  BasicBlockList getList() const;
  void setList(BasicBlockList);

 private:
  BasicBlockList MarkedValues;
};

using LiveVariableList = vector<Value *>;
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
  void addLiveVariable(Value *v);
  bool LiveVariableExists(Value *v);
  void incrementRefCount(Value *v);
  void decrementRefCount(Value *v);

  /*** Branch Related Methods ***/
  void setCorrectlyBranched();
  bool isCorrectlyBranched();
  void setErrorHandlingBlock();
  bool isErrorHandlingBlock();
  void addSucceedingErrorBlock(BasicBlock *B);
  bool isInSucceedingErrorBlock(BasicBlock *B);

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
  void addStoredCallValues(Value *v, CallInst *CI);
  vector<pair<Value *, CallInst *>> getStoredCallValues();
  bool isCallValues(Value *V);
  CallInst *getCallInstForVal(Value *V);
  void addRemoveAlloc(BasicBlock *B, UniqueKey *UK);
  BasicBlockWorkList getRemoveAllocs(BasicBlock *B);
  void setUnconditionalBranched() { unConditionalBranched = true; }
  bool isUnconditionalBranched() { return unConditionalBranched; }

  /*** Original Lists ***/
  BasicBlockList getAllocList() const;
  BasicBlockList getFreeList() const;

  BasicBlockList getErrorRemovedAllocList(BasicBlock *tgt);
  BasicBlockList getErrorAddedFreeList(BasicBlock *tgt);

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

  vector<pair<Value *, CallInst *>> storedCallValues;
  map<BasicBlock *, BasicBlockWorkList> removeAllocs;
  vector<BasicBlock *> succeedingErrorBlocks;

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
  void set(BasicBlock *B);
  BasicBlockInformation *get(BasicBlock *B);
  BasicBlockList getBasicBlockAllocList(BasicBlock *src);
  BasicBlockList getBasicBlockFreeList(BasicBlock *src);
  BasicBlockList getBasicBlockPendingAllocList(BasicBlock *src);

  BasicBlockList getBasicBlockRemoveAllocList(BasicBlock *src, BasicBlock *tgt);
  BasicBlockList getBasicBlockErrorRemovedAllocList(BasicBlock *src, BasicBlock *tgt);
  BasicBlockList getBasicBlockErrorAddedFreeList(BasicBlock *src, BasicBlock *tgt);

  BasicBlockList getBasicBlockDMZList(BasicBlock *src);
  LiveVariableList getLiveVariables(BasicBlock *B);

  BasicBlockList getShrinkedBasicBlockFreeList(BasicBlock *src, BasicBlock *tgt);
  BasicBlockList getShrinkedBasicBlockAllocList(BasicBlock *src, BasicBlock *tgt);

  /*** Mediator ***/
  void CollectInInfo(
      BasicBlock *B, bool isEntryPoint,
      const map<const UniqueKey *, const UniqueKey *> *alias_map);
  void copyAllList(BasicBlock *src, BasicBlock *tgt);
  void copyFreed(BasicBlock *src, BasicBlock *tgt);
  void copyCorrectlyFreed(BasicBlock *src, BasicBlock *tgt);
  void uniteAllocList(BasicBlock *src, BasicBlock *tgt);
  void uniteDMZList(BasicBlock *src, BasicBlock *tgt);
  void intersectFreeList(BasicBlock *src, BasicBlock *tgt);
  void removeAllocatedInError(
      BasicBlock *src, BasicBlock *tgt,
      const map<const UniqueKey *, const UniqueKey *> *alias_map);
  void updateSuccessorBlock(BasicBlock *src);
  void addFreeInfoFromDMZToPreds(BasicBlock *src);
  bool isPredBlockCorrectlyBranched(BasicBlock *B);
  bool checkIfErrorBlock(BasicBlock *B);

  /*** Optimizer ***/
  void shrinkFreedFromAlloc(BasicBlock *B);

 private:
  map<BasicBlock *, BasicBlockInformation> BBMap;
  LiveVariableList intersectLiveVariables(LiveVariableList src,
                                          LiveVariableList tgt);
  bool exists(BasicBlock *B);
};

namespace BasicBlockListOperation {
BasicBlockList intersectList(BasicBlockList src, BasicBlockList tgt);
BasicBlockList uniteList(BasicBlockList src, BasicBlockList tgt);
BasicBlockList diffList(BasicBlockList src, BasicBlockList tgt);
};  // namespace BasicBlockListOperation

}  // namespace ST_free
