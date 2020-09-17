#pragma once
#include "ArgList.hpp"
#include "BasicBlockManager.hpp"
#include "FreedStruct.hpp"
#include "ST_free.hpp"
#include "UniqueKeyManager.hpp"

namespace ST_free {
using FreedStructList = std::vector<FreedStruct *>;
using LocalVarList = std::vector<FreedStruct *>;
using Aliases = std::map<llvm::Value *, llvm::Value *>;

struct FunctionInformation {
 public:
  enum ErrorCollectionMethod { EQUALS, LESS_THAN };
  struct InformationPerErrorCode {
    BasicBlockList alloc_list;
    BasicBlockList free_list;

   public:
    InformationPerErrorCode(){};
    InformationPerErrorCode(BasicBlockList alloc, BasicBlockList free) {
      alloc_list = alloc;
      free_list = free;
    }
  };

  /*** Costructor ***/
  FunctionInformation();
  FunctionInformation(llvm::Function *F);

  /*** Function ***/
  llvm::Function &getFunction();

  /*** EndPoints ***/
  void addEndPoint(llvm::BasicBlock *B, llvm::ReturnInst *RI);
  llvm::BasicBlock *getEndPoint();
  llvm::ReturnInst *getReturnInst();

  void addSuccessBlockInformation(llvm::BasicBlock *B);
  void addErrorBlockInformation(int64_t err, llvm::BasicBlock *B);
  void addErrorBlockAllocInformation(int64_t err, BasicBlockList BList);
  void addErrorBlockFreeInformation(int64_t err, BasicBlockList BList);
  void clearErrorCodeMap() {
    info_per_error_code = std::map<int64_t, InformationPerErrorCode>();
  }
  std::map<int64_t, FunctionInformation::InformationPerErrorCode>
      &getErrorCodeMap() {
    return info_per_error_code;
  }

  /*** FreeValue Related ***/
  ValueInformation *addFreeValueFromDifferentFunction(llvm::BasicBlock *B,
                                                      ValueInformation *VI,
                                                      UniqueKey *UK);
  ValueInformation *addFreeValue(llvm::BasicBlock *B, llvm::Value *V,
                                 llvm::Type *memTy, long num, ParentList plist);
  void addFreeValue(llvm::BasicBlock *B, UniqueKey *UK);
  void incrementFreedRefCount(llvm::BasicBlock *B, llvm::Value *V,
                              llvm::Value *refVal);
  void addFreedStruct(llvm::Type *T, llvm::Value *V, llvm::Instruction *I);
  void addFreedStruct(llvm::BasicBlock *B, llvm::Type *T, llvm::Value *V,
                      llvm::Instruction *I);
  void addFreedStruct(llvm::BasicBlock *B, llvm::Type *T, llvm::Value *V,
                      llvm::Instruction *I, llvm::StructType *parent,
                      ValueInformation *valInfo, bool isInStruct = false);
  void addParentType(llvm::Type *T, llvm::Value *V, llvm::Instruction *I,
                     llvm::StructType *parentTy, int ind);
  FreedStructList getFreedStruct() const;
  bool freedStructExists(FreedStruct *fst);

  /** AllocValue Related ***/
  const UniqueKey *addAllocValue(llvm::BasicBlock *B, llvm::Value *V,
                                 llvm::Type *T, long mem);
  void addAllocValue(llvm::BasicBlock *B, UniqueKey *UK);
  void addPendingArgAlloc(llvm::BasicBlock *B, llvm::Value *V, llvm::Type *T,
                          long mem);
  void addPendingArgAlloc(llvm::BasicBlock *B, UniqueKey *UK);

  /*** Arg related ***/
  ArgList *getArgList() { return &args; }

  /*** Status Related ***/
  bool isUnanalyzed();
  bool isAnalyzed();
  bool isInProgress();
  bool isDirty();
  void setAnalyzed();
  void setInProgress();
  void setDirty();

  /*** BasicBlock Related ***/
  BasicBlockManager *getBasicBlockManager();
  BasicBlockInformation *getBasicBlockInformation(llvm::BasicBlock *B);
  void BBCollectInfo(llvm::BasicBlock &B, bool isEntryPoint);
  void setBasicBlockLoopHeader(llvm::BasicBlock &B, llvm::Loop *L);
  void setBasicBlockLoop(llvm::BasicBlock &B, llvm::Loop *L);
  bool isBasicBlockLoopHeader(llvm::BasicBlock &B);
  bool isBasicBlockLoop(llvm::BasicBlock &B);
  void setLoopInformation(llvm::LoopInfo *LI);
  const llvm::LoopInfo *getLoopInformation();
  BasicBlockList getFreeList(llvm::BasicBlock *B);
  BasicBlockList getAllocList(llvm::BasicBlock *B);
  BasicBlockList getPendingStoreList(llvm::BasicBlock *B);
  bool isFreedInBasicBlock(llvm::BasicBlock *B, llvm::Value *val,
                           llvm::Type *ty, long mem);
  bool isFreedInBasicBlock(llvm::BasicBlock *B, const UniqueKey *UK);
  bool isAllocatedInBasicBlock(llvm::BasicBlock *B, llvm::Value *val,
                               llvm::Type *ty, long mem);
  bool isAllocatedInBasicBlock(llvm::BasicBlock *B, const UniqueKey *UK);
  void addCorrectlyFreedValue(llvm::BasicBlock *, const UniqueKey *UK);
  bool isCorrectlyBranchedFreeValue(llvm::BasicBlock *, llvm::Value *,
                                    llvm::Type *, long mem);
  bool isCorrectlyBranchedFreeValue(llvm::BasicBlock *, const UniqueKey *UK);
  void setCorrectlyBranched(llvm::BasicBlock *B);
  bool isCorrectlyBranched(llvm::BasicBlock *B);
  bool isPredBlockCorrectlyBranched(llvm::BasicBlock *B);
  void setAlias(llvm::Value *srcinfo, llvm::Value *tgtinfo);
  bool aliasExists(llvm::Value *src);
  llvm::Value *getAlias(llvm::Value *src);
  Aliases *getAliasMap() { return &aliasMap; };
  void updateSuccessorBlock(llvm::BasicBlock &B);

  /*** Argument Values ***/
  bool isArgValue(llvm::Value *V);
  long getArgIndex(llvm::Value *V);
  void setArgFree(llvm::Value *V);
  void setArgAlloc(llvm::Value *V);
  void setStructMemberFreed(FreedStruct *fstruct, int64_t num);
  void setStructMemberAllocated(FreedStruct *fstruct, int64_t num);
  std::vector<bool> getStructMemberFreed(llvm::Type *T);
  void copyStructMemberFreed(llvm::Type *T, std::vector<bool> members);
  void setStructArgFree(llvm::Value *V, int64_t num);
  void setStructArgAlloc(llvm::Value *V, int64_t num);
  void setStructMemberArgFreed(llvm::Value *V, ParentList indexes);
  void setStructMemberArgAllocated(llvm::Value *V, int64_t num);
  bool isArgFreed(int64_t num);
  bool isArgAllocated(int64_t num);

  /*** Individual Variable Informations ***/
  ValueInformation *addVariable(llvm::Value *val);
  ValueInformation *addVariable(const UniqueKey *UK, llvm::Value *val,
                                ParentList plist);
  // ValueInformation * getValueInfo(Value * val);
  ValueInformation *getValueInfo(llvm::Value *val, llvm::Type *ty, long num);
  ValueInformation *getValueInfo(const UniqueKey *UK);
  bool variableExists(llvm::Value *);
  void addLocalVar(llvm::BasicBlock *, llvm::Type *, llvm::Value *,
                   llvm::Instruction *);
  void addLocalVar(llvm::BasicBlock *, llvm::Type *, llvm::Value *,
                   llvm::Instruction *, ParentList P, ValueInformation *);
  LocalVarList getLocalVar() const;
  void addBasicBlockLiveVariable(llvm::BasicBlock *B, llvm::Value *);
  bool localVarExists(llvm::Type *);
  // void incrementRefCount(Value *V, Type *T, long mem, Value *ref);
  bool isLiveInBasicBlock(llvm::BasicBlock *B, llvm::Value *val);

  /*** Debugging ***/
  void printVal() { VManage.print(); }
  size_t getVManageSize() { return VManage.getSize(); }

  /*** Func Ptr related ***/
  void addFunctionPointerInfo(llvm::Value *val, llvm::Function *func);
  std::vector<llvm::Function *> getPointedFunctions(llvm::Value *val);

  /*** UniqueKeys ***/
  UniqueKeyManager *getUniqueKeyManager() { return &UKManage; }

  /*** Aliased Type ***/
  void addAliasedType(llvm::Value *V, llvm::Type *T);
  llvm::Type *getAliasedType(llvm::Value *V);
  bool aliasedTypeExists(llvm::Value *V);

  /*** get Allocated ***/
  BasicBlockList getAllocatedInReturn();
  BasicBlockList getAllocatedInSuccess();
  BasicBlockList getAllocatedInError(int errcode,
                                     ErrorCollectionMethod method = LESS_THAN);
  bool errorCodeExists(int errcode);
  bool errorCodeLessThanExists(int errcode);

  /*** get Freed ***/
  BasicBlockList getFreedInReturn();
  BasicBlockList getFreedInSuccess();
  BasicBlockList getFreedInError(int errcode,
                                 ErrorCollectionMethod method = LESS_THAN);

  /*** get Freed ***/
  BasicBlockList getPendingStoreInReturn();

  /*** Pending Aliased Alloc Operator ***/
  void addPendingAliasedAlloc(const UniqueKey *UK);
  bool checkAndPopPendingAliasedAlloc(const UniqueKey *UK);

  /*** UniqueKey alias ***/
  void setUniqueKeyAlias(const UniqueKey *src, const UniqueKey *dest);
  bool hasUniqueKeyAlias(const UniqueKey *src);
  const UniqueKey *getUniqueKeyAlias(const UniqueKey *src);
  const std::map<const UniqueKey *, const UniqueKey *> *getUniqueKeyAliasMap();

 private:
  enum AnalysisStat { UNANALYZED, IN_PROGRESS, DIRTY, ANALYZED };

  /*** Private Variables ***/
  static UniqueKeyManager UKManage;
  llvm::Function *F;
  AnalysisStat stat;
  ArgList args;
  llvm::BasicBlock *endPoint;
  llvm::ReturnInst *retInst;

  LocalVarList localVariables;
  FreedStructList freedStruct;
  BasicBlockManager BBManage;
  ValueManager VManage;
  std::map<llvm::Value *, std::vector<llvm::Function *>> funcPtr;
  std::map<llvm::Value *, llvm::Type *> aliasedType;
  Aliases aliasMap;
  bool dirty;

  /*** Analysis results from loop analyzer ***/
  const llvm::LoopInfo *loop_info;

  /*** Return Error Code Map ***/
  std::map<int64_t, InformationPerErrorCode> info_per_error_code;

  /*** UniqueKey Alias Map for lazy evaluation ***/
  std::vector<const UniqueKey *> pending_alloc_store;
  std::map<const UniqueKey *, const UniqueKey *> allocated_alias;

  /*** Private Methods ***/
  AnalysisStat getStat();
  void setStat(AnalysisStat);
};

class FunctionManager {
 public:
  bool exists(llvm::Function *);
  FunctionInformation *getElement(llvm::Function *F);

 private:
  std::map<llvm::Function *, FunctionInformation *> func_map;
};
}  // namespace ST_free
