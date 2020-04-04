#pragma once
#include "ST_free.hpp"
#include "UniqueKeyManager.hpp"
#include "ValueInformation.hpp"

namespace ST_free {
using BasicBlockList = vector<const UniqueKey *>;
class BasicBlockWorkList {
 public:
  BasicBlockWorkList();
  BasicBlockWorkList(const BasicBlockList);
  // void add(Value * v, Type * t, long mem);
  void add(const UniqueKey *UK);
  bool exists(const UniqueKey *UK);
  bool typeExists(Type *T);
  bool valueExists(Value *V);
  bool fieldExists(Type *T, long ind);
  const UniqueKey *getUKFromValue(Value *V);
  // bool exists(Value * v, Type *t, long mem);
  BasicBlockList getList() const;
  void setList(BasicBlockList);

 private:
  BasicBlockList MarkedValues;
};

using LiveVariableList = vector<Value *>;
class BasicBlockInformation {
 public:
  BasicBlockInformation();
  BasicBlockInformation(const BasicBlockInformation &);
  void initLists(const BasicBlockInformation &);
  /*** Free Related Methods ***/
  // void addFree(Value * v, Type * ty, long mem);
  void addFree(const UniqueKey *UK);
  // bool FreeExists(Value *v, Type * ty, long mem);
  bool FreeExists(const UniqueKey *UK);
  void setFreeList(BasicBlockList);
  /*** Alloc Related Methods ***/
  // void addAlloc(Value *v, Type * ty, long mem);
  void addAlloc(const UniqueKey *UK);
  // bool AllocExists(Value *v, Type *ty, long mem);
  bool AllocExists(const UniqueKey *UK);
  void setAllocList(BasicBlockList);
  void setDMZList(BasicBlockList);
  BasicBlockWorkList getDMZList() const;
  void addPendingArgAlloc(const UniqueKey *UK);
  BasicBlockWorkList getPendingArgAllocList() const;
  void setPendingArgAllocList(BasicBlockList);
  /*** Live Variable Methods ***/
  void setLiveVariables(LiveVariableList);
  void addLiveVariable(Value *v);
  bool LiveVariableExists(Value *v);
  void incrementRefCount(Value *v);
  void decrementRefCount(Value *v);
  /*** Correct Branch Freed Methods ***/
  void setCorrectlyBranched();
  bool isCorrectlyBranched();
  void setLoopBlock();
  bool isLoopBlock();
  void setErrorHandlingBlock();
  bool isErrorHandlingBlock();
  void addSucceedingErrorBlock(BasicBlock *B);
  bool isInSucceedingErrorBlock(BasicBlock *B);
  /*** CorrectlyFreed ***/
  // void addCorrectlyFreedValue(Value * V, Type * T, long mem);
  void addCorrectlyFreedValue(const UniqueKey *UK);
  // bool CorrectlyFreedValueExists(Value * V, Type * T, long mem);
  bool CorrectlyFreedValueExists(const UniqueKey *UK);
  BasicBlockWorkList getCorrectlyFreedValues() const;
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

 private:
  /*** BasicBlock Lists ***/
  BasicBlockWorkList freeList;
  BasicBlockWorkList allocList;
  BasicBlockWorkList pendingArgStoreList;
  BasicBlockWorkList correctlyFreed;
  BasicBlockWorkList dmzList;
  LiveVariableList liveVariables;
  vector<pair<Value *, CallInst *>> storedCallValues;
  map<BasicBlock *, BasicBlockWorkList> removeAllocs;
  vector<BasicBlock *> succeedingErrorBlocks;
  /*** BasicBlock Status ***/
  bool correctlyBranched;
  bool predCorrectlyBranched;
  bool loopBlock;
  bool errorHandlingBlock;
  bool unConditionalBranched;
};
class BasicBlockManager {
 public:
  /*** getter ***/
  void set(BasicBlock *B);
  BasicBlockInformation *get(BasicBlock *B);
  BasicBlockList getBasicBlockFreeList(BasicBlock *src);
  BasicBlockList getBasicBlockAllocList(BasicBlock *src);
  BasicBlockList getBasicBlockDMZList(BasicBlock *src);
  BasicBlockList getBasicBlockPendingAllocList(BasicBlock *src);
  BasicBlockList getBasicBlockRemoveAllocList(BasicBlock *src, BasicBlock *tgt);
  LiveVariableList getLiveVariables(BasicBlock *B);
  /*** Mediator ***/
  void CollectInInfo(BasicBlock *B, bool isEntryPoint,
                     map<Value *, Value *> *alias_map);
  void copyAllList(BasicBlock *src, BasicBlock *tgt);
  void copyFreed(BasicBlock *src, BasicBlock *tgt);
  void copyCorrectlyFreed(BasicBlock *src, BasicBlock *tgt);
  void uniteAllocList(BasicBlock *src, BasicBlock *tgt);
  void uniteDMZList(BasicBlock *src, BasicBlock *tgt);
  void intersectFreeList(BasicBlock *src, BasicBlock *tgt);
  void removeAllocatedInError(BasicBlock *src, BasicBlock *tgt,
                              map<Value *, Value *> *alias_map);
  void updateSuccessorBlock(BasicBlock *src);
  void addFreeInfoFromDMZToPreds(BasicBlock *src);
  bool isPredBlockCorrectlyBranched(BasicBlock *B);
  bool checkIfErrorBlock(BasicBlock *B);

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
