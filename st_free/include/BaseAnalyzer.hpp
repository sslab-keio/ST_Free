#pragma once
#include "ArgList.hpp"
#include "BasicBlockManager.hpp"
#include "FunctionManager.hpp"
#include "RelationshipInformation.hpp"
#include "ST_free.hpp"
#include "StructInformation.hpp"
#include "UniqueKeyManager.hpp"
#include "ValueInformation.hpp"
#include "determinator.hpp"
#include "statList.hpp"
#include "support_funcs.hpp"
#include "LoopManager.hpp"

#define WORKLIST_MAX_INTERATION 3

namespace ST_free {
class BaseAnalyzer {
 public:
  BaseAnalyzer() {}
  BaseAnalyzer(llvm::Function *func, StructManager *stm, const llvm::DataLayout *dl) {
    FEle = identifier.getElement(func);
    stManage = stm;
    dat_layout = dl;
    InstAnalysisMap[llvm::Instruction::Alloca] = &BaseAnalyzer::analyzeAllocaInst;
    InstAnalysisMap[llvm::Instruction::Call] = &BaseAnalyzer::analyzeCallInst;
    InstAnalysisMap[llvm::Instruction::Store] = &BaseAnalyzer::analyzeStoreInst;
    InstAnalysisMap[llvm::Instruction::Br] = &BaseAnalyzer::analyzeBranchInst;
    InstAnalysisMap[llvm::Instruction::Switch] = &BaseAnalyzer::analyzeSwitchInst;
    InstAnalysisMap[llvm::Instruction::Ret] = &BaseAnalyzer::analyzeReturnInst;
    InstAnalysisMap[llvm::Instruction::BitCast] = &BaseAnalyzer::analyzeBitCastInst;
    InstAnalysisMap[llvm::Instruction::GetElementPtr] =
        &BaseAnalyzer::analyzeGetElementPtrInst;
  }
  BaseAnalyzer(StructManager *stm, const llvm::DataLayout *dl) {
    stManage = stm;
    dat_layout = dl;
    InstAnalysisMap[llvm::Instruction::Alloca] = &BaseAnalyzer::analyzeAllocaInst;
    InstAnalysisMap[llvm::Instruction::Call] = &BaseAnalyzer::analyzeCallInst;
    InstAnalysisMap[llvm::Instruction::Store] = &BaseAnalyzer::analyzeStoreInst;
    InstAnalysisMap[llvm::Instruction::Br] = &BaseAnalyzer::analyzeBranchInst;
    InstAnalysisMap[llvm::Instruction::Switch] = &BaseAnalyzer::analyzeSwitchInst;
    InstAnalysisMap[llvm::Instruction::Ret] = &BaseAnalyzer::analyzeReturnInst;
    InstAnalysisMap[llvm::Instruction::BitCast] = &BaseAnalyzer::analyzeBitCastInst;
    InstAnalysisMap[llvm::Instruction::GetElementPtr] =
        &BaseAnalyzer::analyzeGetElementPtrInst;
  }
  void analyze(llvm::Function &F);
  void analyzeAdditionalUnknowns(llvm::Function &F);
  void analyzeDifferentFunc(llvm::Function &);

  void setLoopManager(LoopManager *lm) {loop_manager = lm;}
  LoopManager *getLoopManager() {return loop_manager;}

 protected:
  /*** Class-protected Struct Element ***/
  struct collectedInfo {
    bool isStructRelated;
    long index;
    llvm::Value *freeValue;
    llvm::Type *memType;
    llvm::StructType *parentType;
    ParentList indexes;
    collectedInfo() {
      isStructRelated = false;
      index = ROOT_INDEX;
      freeValue = NULL;
      memType = NULL;
      parentType = NULL;
    }
  };
  /*** getter/setter ***/
  FunctionManager *getFunctionManager() { return &identifier; };
  FunctionInformation *getFunctionInformation() { return FEle; };
  void setFunctionInformation(FunctionInformation *FInfo) { FEle = FInfo; };
  StructManager *getStructManager() { return stManage; };
  void setStructManager(StructManager *stManager) { stManage = stManager; };
  TypeRelationManager *getTypeRelationManager() {
    return stManage->getTypeRelationManager();
  };
  void setDataLayout(llvm::DataLayout *dl) { dat_layout = dl; }
  const llvm::DataLayout *getDataLayout() { return dat_layout; }
  const llvm::StructLayout *getStructLayout(llvm::StructType *STy) {
    return dat_layout->getStructLayout(STy);
  }

  /*** Availability Analysis ***/
  virtual void checkAvailability();

  /*** Instruction Analysis ***/
  virtual void analyzeInstructions(llvm::BasicBlock &B);
  virtual void analyzeAllocaInst(llvm::Instruction *AI, llvm::BasicBlock &B);
  virtual void analyzeStoreInst(llvm::Instruction *SI, llvm::BasicBlock &B);
  virtual void analyzeCallInst(llvm::Instruction *CI, llvm::BasicBlock &B);
  virtual void analyzeBranchInst(llvm::Instruction *BI, llvm::BasicBlock &B);
  virtual void analyzeSwitchInst(llvm::Instruction *SwI, llvm::BasicBlock &B);
  virtual void analyzeBitCastInst(llvm::Instruction *BCI, llvm::BasicBlock &B);
  virtual void analyzeICmpInst(llvm::Instruction *ICI, llvm::BasicBlock &B);
  virtual void analyzeReturnInst(llvm::Instruction *RI, llvm::BasicBlock &B);
  virtual void analyzeGetElementPtrInst(llvm::Instruction *RI, llvm::BasicBlock &B);
  bool isReturnFunc(llvm::Instruction *I);

  /*** add Value ***/
  virtual void addFree(llvm::Value *V, llvm::CallInst *CI, llvm::BasicBlock *B,
                       bool isAlias = false,
                       ParentList additionalParents = ParentList());
  void addAlloc(llvm::CallInst *CI, llvm::BasicBlock *B);
  void addLocalVariable(llvm::BasicBlock *B, llvm::Type *T, llvm::Value *V, llvm::Instruction *I,
                        ParentList P);
  void addPointerLocalVariable(llvm::BasicBlock *B, llvm::Type *T, llvm::Value *V, llvm::Instruction *I,
                               ParentList P);
  void collectStructMemberFreeInfo(llvm::Instruction *I, struct collectedInfo &info,
                                   ParentList &additionalParents);
  void collectStructFreeInfo(llvm::Instruction *I, struct collectedInfo &info);
  void collectOptimizedStructFreeInfo(llvm::Instruction *I,
                                      struct collectedInfo &info);
  void collectSimpleFreeInfo(llvm::Instruction *I, struct collectedInfo &info);
  void addNestedFree(llvm::Value *V, llvm::CallInst *CI, llvm::BasicBlock *B,
                     struct collectedInfo &I, ParentList &additionalParents);
  void addRefcountedFree(llvm::Value* V, llvm::CallInst *CI, llvm::BasicBlock *B);

  /*** Argument Status ***/
  void copyArgStatus(llvm::Function &Func, llvm::CallInst *CI, llvm::BasicBlock &B);
  void copyArgStatusRecursively(llvm::Function &Func, llvm::CallInst *CI, llvm::BasicBlock &B,
                                llvm::Value *arg, ArgStatus *ArgStat, int ind,
                                llvm::Type *parentType, ParentList plist,
                                bool isFirst = false);
  void copyAllocatedStatus(llvm::Function &Func, llvm::CallInst *CI, llvm::BasicBlock &B);
  void copyFreeStatus(llvm::Function &Func, llvm::CallInst *CI, llvm::BasicBlock &B);
  void evaluatePendingStoredValue(llvm::Function &Func, llvm::CallInst *CI, llvm::BasicBlock &B);

  /*** Branch Instruction(Error Code Analysis) ***/
  bool isCorrectlyBranched(llvm::BranchInst *BI);
  void analyzeErrorCode(llvm::BranchInst *BI, llvm::ICmpInst *ICI, llvm::BasicBlock &B);
  void analyzeErrorCode(llvm::SwitchInst *SwI, llvm::ICmpInst *ICI, llvm::BasicBlock *tgtBlock, llvm::BasicBlock &B);
  void analyzeNullCheck(llvm::BranchInst *BI, llvm::ICmpInst *ICI, llvm::BasicBlock &B);
  void analyzeErrorCheckFunction(llvm::BranchInst *BI, llvm::CallInst *CI, llvm::BasicBlock &B);
  BasicBlockWorkList getErrorValues(llvm::Instruction *I, llvm::BasicBlock &B, int errcode);
  BasicBlockWorkList getSuccessValues(llvm::Instruction *I, llvm::BasicBlock &B);
  bool errorCodeExists(llvm::Instruction *I, llvm::BasicBlock &B, int errcode);
  llvm::Value *getComparedValue(llvm::ICmpInst *ICI);
  llvm::Value *decodeComparedValue(llvm::Value *V);
  ParentList decodeErrorTypes(llvm::Value *V);
  llvm::Type *getComparedType(llvm::Value *V, llvm::BasicBlock &B);
  int getErrorOperand(llvm::ICmpInst *ICI);
  BasicBlockList getErrorAllocInCalledFunction(llvm::CallInst *CI, int errcode);
  BasicBlockList getSuccessAllocInCalledFunction(llvm::CallInst *CI);
  BasicBlockList getErrorFreeInCalledFunction(llvm::CallInst *CI, int errcode);
  BasicBlockList getSuccessFreeInCalledFunction(llvm::CallInst *CI);
  void buildReturnValueInformation();
  void checkErrorInstruction(llvm::Value *v, std::vector<llvm::Instruction*> visited_inst = std::vector<llvm::Instruction*>());
  void checkErrorCodeAndAddBlock(llvm::Instruction *I, llvm::BasicBlock *B, llvm::Value *inval, std::vector<llvm::Instruction*> visited_inst);

  /*** Store Instruction related funtions ***/
  llvm::CallInst *getStoreFromCall(llvm::StoreInst *SI);
  bool isStoreToStructMember(llvm::StoreInst *SI);
  bool isStoreFromStructMember(llvm::StoreInst *SI);
  bool isStoreToStruct(llvm::StoreInst *SI);
  llvm::StructType *getStoreeStruct(llvm::StoreInst *SI);
  llvm::StructType *getStorerStruct(llvm::StoreInst *SI);
  bool isStoreFromStruct(llvm::StoreInst *SI);
  void checkAndChangeActualAuthority(llvm::StoreInst *SI);
  void changeAuthority(llvm::StoreInst *SI, llvm::CastInst *CI, llvm::GetElementPtrInst *GEle);
  bool isDirectStoreFromAlloc(llvm::StoreInst *SI);
  bool isAllocCast(llvm::CastInst *CI);
  bool isCastToVoid(llvm::CastInst *CI);
  std::vector<std::pair<llvm::Type *, long>> decodeGEPInst(llvm::GetElementPtrInst *GEle);
  std::vector<std::string> decodeDirectoryName(std::string str);
  void getStructParents(llvm::Instruction *I, std::vector<std::pair<llvm::Type *, int>> &typeList);

  /*** Determinator ***/
  long getMemberIndiceFromByte(llvm::StructType *STy, uint64_t byte);
  bool isStructEleAlloc(llvm::Instruction *);
  bool isStructEleFree(llvm::Instruction *);
  bool isStructFree(llvm::Instruction *);
  bool isOptimizedStructFree(llvm::Instruction *I);
  bool isOptimizedStructEleFree(llvm::Instruction *);
  llvm::Type *getOptimizedStructFree(llvm::Instruction *I);
  llvm::Value *getStructFreedValue(llvm::Instruction *val, bool isUserDefCalled = false);
  llvm::Value *getCalledStructFreedValue(llvm::Instruction *val);
  bool isHeapValue(llvm::Value *);
  bool isFuncPointer(llvm::Type *t);
  bool isBidirectionalAlias(llvm::Value *V);
  llvm::GetElementPtrInst *getAllocStructEleInfo(llvm::Instruction *);
  llvm::GetElementPtrInst *getFreeStructEleInfo(llvm::Instruction *);
  llvm::GetElementPtrInst *getStoredStructEle(llvm::StoreInst *SI);
  llvm::GetElementPtrInst *getStoredStruct(llvm::StoreInst *SI);
  llvm::Type *getStructType(llvm::Instruction *val);
  llvm::Value *getFreedValue(llvm::Instruction *val);
  llvm::Value *getAllocatedValue(llvm::Instruction *I);
  bool isCallInstReturnValue(llvm::Value *V);
  bool isAllocStoredInSameBasicBlock(llvm::Value *V, llvm::BasicBlock *B);

  llvm::Function* getCalledFunction(llvm::CallInst *CI);

  /*** Support Methods ***/
  std::vector<long> getValueIndices(llvm::GetElementPtrInst *inst);
  llvm::GetElementPtrInst *getRootGEle(llvm::GetElementPtrInst *GEle);
  llvm::Type *extractResultElementType(llvm::GetElementPtrInst *GEle);
  void reversePropagateErrorBlockFreeInfo();

  /*** connector with struct manager***/
  bool isAuthorityChained(ParentList);

  /*** find icmp ***/
  llvm::ICmpInst *findAllocICmp(llvm::Instruction *I);

  /*** MethodMap ***/
  std::map<unsigned, void (ST_free::BaseAnalyzer::*)(llvm::Instruction *, llvm::BasicBlock &)>
      InstAnalysisMap;

 private:
  /*** private methods ***/
  void __recursiveReversePropagateErrorBlockFreeInfo(llvm::BasicBlock *B);
  /*** Managers and DataLayouts ***/
  FunctionManager identifier;
  StructManager *stManage;
  const llvm::DataLayout *dat_layout;
  LoopManager *loop_manager;

  /*** Current Function/ Stacked Functions ***/
  FunctionInformation *FEle;
  std::stack<llvm::Function *> functionStack;
};
}  // namespace ST_free
