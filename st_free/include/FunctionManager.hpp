#pragma once
#include "ST_free.hpp"
#include "BasicBlockManager.hpp"
#include "ArgList.hpp"
#include "UniqueKeyManager.hpp"
#include "FreedStruct.hpp"

namespace ST_free {
    using FreedStructList = vector<FreedStruct *>;
    using LocalVarList = vector<FreedStruct *>;
    struct FunctionInformation {
        public:
            /*** Costructor ***/
            FunctionInformation();
            FunctionInformation(Function *F);
            /*** Function ***/
            Function& getFunction();
            /*** EndPoints ***/
            void addEndPoint(BasicBlock *B);
            vector<BasicBlock *> getEndPoint() const;
            void addSuccessBlock(BasicBlock *B);
            vector<BasicBlock *> getSuccessBlock() const;
            void addErrorBlock(int64_t err, BasicBlock *B);
            vector<pair<int64_t, BasicBlock *>> getErrorBlock() const;
            /*** FreeValue Related ***/
            // void addFreeValue(BasicBlock *B, Value *V);
            // void addFreeValue(BasicBlock *B, Value *V, Type *memTy, Type * stTy, long num);
            ValueInformation* addFreeValue(BasicBlock *B, Value *V, Type *memTy, Type * stTy, long num, ParentList plist);
            void incrementFreedRefCount(BasicBlock *B, Value *V, Value *refVal);
            void addFreedStruct(Type *T, Value *V, Instruction *I);
            void addFreedStruct(BasicBlock *B, Type *T, Value *V, Instruction *I);
            void addFreedStruct(BasicBlock *B, Type *T, Value *V, Instruction *I, StructType *parent, ValueInformation *valInfo, bool isInStruct = false);
            // void addFreedStructValue();
            void addParentType(Type *T, Value *V, Instruction *I, StructType *parentTy, int ind);
            FreedStructList getFreedStruct() const;
            bool freedStructExists(FreedStruct *fst);
            /** AllocValue Related ***/
            void addAllocValue(BasicBlock *B, Value *V, Type *T, long mem);
            void addAllocValue(BasicBlock *B, UniqueKey *UK);
            /*** Arg related ***/
            ArgList* getArgList(){return &args;}
            /*** Status Related ***/
            bool isUnanalyzed();
            bool isAnalyzed();
            bool isInProgress();
            void setAnalyzed();
            void setInProgress();
            /*** BasicBlock Related ***/
            BasicBlockInformation * getBasicBlockInformation(BasicBlock *B);
            void BBCollectInfo(BasicBlock& B, bool isEntryPoint);
            BasicBlockList getFreeList(BasicBlock *B);
            BasicBlockList getAllocList(BasicBlock *B);
            bool isFreedInBasicBlock(BasicBlock *B, Value * val, Type * ty, long mem);
            bool isFreedInBasicBlock(BasicBlock *B, const UniqueKey *UK);
            bool isAllocatedInBasicBlock(BasicBlock *B, Value * val, Type * ty, long mem);
            bool isAllocatedInBasicBlock(BasicBlock *B, const UniqueKey *UK);
            void addCorrectlyFreedValue(BasicBlock *, const UniqueKey *UK);
            bool isCorrectlyBranchedFreeValue(BasicBlock *, Value *, Type *, long mem);
            bool isCorrectlyBranchedFreeValue(BasicBlock *, const UniqueKey *UK);
            void setCorrectlyBranched(BasicBlock *B);
            bool isCorrectlyBranched(BasicBlock *B);
            bool isPredBlockCorrectlyBranched(BasicBlock *B);
            void setAliasInBasicBlock(BasicBlock *B, Value *srcinfo, Value *tgtinfo);
            bool aliasExists(BasicBlock *B, Value * src);
            Value * getAlias(BasicBlock *B, Value *src);
            // void copyCorrectlyFreedValueInLoop(BasicBlock &B);
            void updateSuccessorBlock(BasicBlock &B);
            /*** Loop Related ***/
            void setLoopInfo(LoopInfo * li);
            void setLoopBlock(BasicBlock &B);
            bool isLoopBlock(BasicBlock &B);
            /*** Argument Values ***/
            bool isArgValue(Value *V);
            void setArgFree(Value *V);
            void setArgAlloc(Value *V);
            void setStructMemberFreed(FreedStruct * fstruct, int64_t num);
            void setStructMemberAllocated(FreedStruct * fstruct, int64_t num);
            vector<bool> getStructMemberFreed(Type * T);
            void copyStructMemberFreed(Type * T, vector<bool> members);
            void setStructArgFree(Value *V, int64_t num);
            void setStructArgAlloc(Value *V, int64_t num);
            void setStructMemberArgFreed(Value *V, ParentList indexes);
            void setStructMemberArgAllocated(Value *V, int64_t num);
            bool isArgFreed(int64_t num);
            bool isArgAllocated(int64_t num);
            /*** Individual Variable Informations ***/
            ValueInformation * addVariable(Value * val);
            // ValueInformation * addVariable(Value * val, Type * memType, Type *parType, long num);
            // ValueInformation * addVariable(const UniqueKey *UK, Value * val, Type * memType, Type *parType, long num);
            ValueInformation * addVariable(const UniqueKey *UK, Value * val, Type * memType, Type *parType, long num, ParentList plist);
			// ValueInformation * getValueInfo(Value * val);
			ValueInformation * getValueInfo(Value * val, Type * ty, long num);
			ValueInformation * getValueInfo(const UniqueKey *UK);
            bool variableExists(Value *);
            void addLocalVar(BasicBlock *, Type *, Value *, Instruction *);
            void addLocalVar(BasicBlock *, Type *, Value *, Instruction *, ParentList P, ValueInformation *);
            LocalVarList getLocalVar() const;
            void addBasicBlockLiveVariable(BasicBlock *B, Value *);
            bool localVarExists(Type *);
            // void incrementRefCount(Value *V, Type *T, long mem, Value *ref);
            bool isLiveInBasicBlock(BasicBlock *B, Value *val);
            /*** Debugging ***/
            void printVal(){VManage.print();}
            /*** Func Ptr related ***/
            void addFunctionPointerInfo(Value *val, Function *func);
            vector<Function *> getPointedFunctions(Value *val);
            /*** UniqueKeys ***/
            UniqueKeyManager* getUniqueKeyManager(){return &UKManage;}
            /*** Aliased Type ***/
            void addAliasedType(Value* V, Type* T);
            Type* getAliasedType(Value *V);
            bool aliasedTypeExists(Value *V);
            /*** get Allocated ***/
            BasicBlockList getAllocatedInReturn();
            BasicBlockList getAllocatedInSuccess();
            BasicBlockList getAllocatedInError(int errcode);
            BasicBlockList uniteList(BasicBlockList src, BasicBlockList tgt);
            BasicBlockList diffList(BasicBlockList src, BasicBlockList tgt);
        private:
            /*** Private Variables ***/
            static UniqueKeyManager UKManage;
            Function *F;
            int stat;
            ArgList args;
            vector<BasicBlock *> endPoint;
            vector<BasicBlock *> successBlock;
            vector<pair<int64_t, BasicBlock *>> errorBlock;
            LocalVarList localVariables;
            FreedStructList freedStruct;
            BasicBlockManager BBManage;
            ValueManager VManage;
            LoopInfo * LoopI;
            map<Value *, vector<Function *>> funcPtr;
            map<Value *, Type*> aliasedType;
            /*** Private Methods ***/
            int getStat();
            void setStat(int);
    };
    class FunctionManager {
        public:
            bool exists(Function *);
            FunctionInformation* getElement(Function *F);
        private:
            map<Function*, FunctionInformation*> func_map;
    };
}
