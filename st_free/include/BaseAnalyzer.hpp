#pragma once
#include "ST_free.hpp"
#include "FunctionManager.hpp"
#include "statList.hpp"
#include "ArgList.hpp"
#include "BasicBlockManager.hpp"
#include "ValueInformation.hpp"
#include "StructInformation.hpp"
#include "determinator.hpp"
#include "support_funcs.hpp"
#include "LoopManager.hpp"
#include "RelationshipInformation.hpp"
#include "UniqueKeyManager.hpp"

namespace ST_free {
    class BaseAnalyzer {
        public:
            BaseAnalyzer(){
            }
            BaseAnalyzer(Function *func, StructManager *stm, LoopManager *lmap, const DataLayout *dl) {
                FEle = identifier.getElement(func);
                loopmap = lmap;
                stManage = stm;
                FEle->setLoopInfo(loopmap->get(func));
                dat_layout = dl;
                InstAnalysisMap[Instruction::Call] = &BaseAnalyzer::analyzeCallInst;
                InstAnalysisMap[Instruction::Store] = &BaseAnalyzer::analyzeStoreInst;
                InstAnalysisMap[Instruction::Br] = &BaseAnalyzer::analyzeBranchInst;
                InstAnalysisMap[Instruction::Ret] = &BaseAnalyzer::analyzeReturnInst;
                InstAnalysisMap[Instruction::BitCast] = &BaseAnalyzer::analyzeBitCastInst;
                InstAnalysisMap[Instruction::GetElementPtr] = &BaseAnalyzer::analyzeGetElementPtrInst;
            }
            BaseAnalyzer(StructManager *stm, LoopManager *lmap, const DataLayout *dl){
                loopmap = lmap;
                stManage = stm;
                dat_layout = dl;
                InstAnalysisMap[Instruction::Call] = &BaseAnalyzer::analyzeCallInst;
                InstAnalysisMap[Instruction::Store] = &BaseAnalyzer::analyzeStoreInst;
                InstAnalysisMap[Instruction::Br] = &BaseAnalyzer::analyzeBranchInst;
                InstAnalysisMap[Instruction::Ret] = &BaseAnalyzer::analyzeReturnInst;
                InstAnalysisMap[Instruction::BitCast] = &BaseAnalyzer::analyzeBitCastInst;
                InstAnalysisMap[Instruction::GetElementPtr] = &BaseAnalyzer::analyzeGetElementPtrInst;
            }
            void analyze(Function &F);
            void analyzeAdditionalUnknowns(Function &F);
            void analyzeDifferentFunc(Function &);
        protected:
            /*** Class-protected Struct Element ***/
            struct collectedInfo {
                bool isStructRelated;
                long index;
                Value* freeValue;
                Type* memType;
                StructType* parentType;
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
            FunctionManager* getFunctionManager(){return &identifier;};
            LoopManager* getLoopManager(){return loopmap;};
            FunctionInformation* getFunctionInformation(){return FEle;};
            void setFunctionInformation(FunctionInformation * FInfo){FEle = FInfo;};
            StructManager* getStructManager(){return stManage;};
            void setStructManager(StructManager *stManager){stManage = stManager;};
            TypeRelationManager* getTypeRelationManager(){return stManage->getTypeRelationManager();};
            void setDataLayout(DataLayout* dl) {dat_layout = dl;}
            const DataLayout* getDataLayout() {return dat_layout;}
            const StructLayout* getStructLayout(StructType *STy) {return dat_layout->getStructLayout(STy);}
            /*** Availability Analysis ***/
            virtual void checkAvailability();
            /*** Instruction Analysis ***/
            virtual void analyzeInstructions(BasicBlock &B);
            virtual void analyzeAllocaInst(Instruction * AI, BasicBlock &B);
            virtual void analyzeStoreInst(Instruction* SI, BasicBlock &B);
            virtual void analyzeCallInst(Instruction *CI, BasicBlock &B);
            virtual void analyzeBranchInst(Instruction *BI, BasicBlock &B);
            virtual void analyzeBitCastInst(Instruction *BCI, BasicBlock &B);
            virtual void analyzeICmpInst(Instruction *ICI, BasicBlock &B);
            virtual void analyzeReturnInst(Instruction *RI, BasicBlock &B);
            virtual void analyzeGetElementPtrInst(Instruction *RI, BasicBlock &B);
            bool isReturnFunc(Instruction *I);
            /*** add Value ***/
            virtual void addFree(Value * V, CallInst *CI, BasicBlock *B, bool isAlias = false, ParentList additionalParents = ParentList());
            void addAlloc(CallInst *CI, BasicBlock *B);
            void addLocalVariable(BasicBlock * B, Type * T, Value * V, Instruction * I, ParentList P);
            void addPointerLocalVariable(BasicBlock *B, Type * T, Value * V, Instruction * I, ParentList P);
            void collectStructMemberFreeInfo(Instruction *I, struct collectedInfo &info, ParentList &additionalParents);
            void collectStructFreeInfo(Instruction *I, struct collectedInfo &info);
            void collectOptimizedStructFreeInfo(Instruction *I, struct collectedInfo &info);
            void collectSimpleFreeInfo(Instruction *I, struct collectedInfo &info);
            void addNestedFree(Value *V, CallInst *CI, BasicBlock *B, struct collectedInfo &I, ParentList &additionalParents);
            /*** Argument Status ***/
            void copyArgStatus(Function &Func, CallInst *CI, BasicBlock &B);
            void copyArgStatusRecursively(Function &Func, CallInst *CI, BasicBlock &B, Value* arg, ArgStatus *ArgStat, int ind, Type* parentType, ParentList plist, bool isFirst = false);
            void copyAllocatedStatus(Function &Func, BasicBlock &B);
            void copyFreeStatus(Function &Func, BasicBlock &B);
            /*** Branch Instruction(Correctly Branched) ***/
            bool isCorrectlyBranched(BranchInst * BI);
            BasicBlockWorkList getErrorValues(Instruction *I, BasicBlock &B, int errcode);
            Value* getComparedValue(ICmpInst *ICI);
            Value* decodeComparedValue(Value *V);
            ParentList decodeErrorTypes(Value *V);
            Type* getComparedType(Value *V, BasicBlock &B);
            int getErrorOperand(ICmpInst *ICI);
            BasicBlockList getErrorAllocInCalledFunction(CallInst *CI, int errcode);
            void checkErrorCodeAndAddBlock(Instruction *I, BasicBlock *B,Value *inval);
            void checkErrorInstruction(Value *v);
            /*** Store Instruction related funtions ***/
            CallInst *getStoreFromCall(StoreInst *SI);
            bool isStoreToStructMember(StoreInst *SI);
            bool isStoreFromStructMember(StoreInst *SI);
            bool isStoreToStruct(StoreInst *SI);
            StructType* getStoreeStruct(StoreInst *SI);
            StructType* getStorerStruct(StoreInst *SI);
            bool isStoreFromStruct(StoreInst *SI);
            void checkAndChangeActualAuthority(StoreInst *SI);
            void changeAuthority(StoreInst *SI, CastInst *CI, GetElementPtrInst *GEle);
            bool isAllocCast(CastInst *CI);
            bool isCastToVoid(CastInst *CI);
            vector<pair<Type*, long>> decodeGEPInst(GetElementPtrInst *GEle);
            vector<string> decodeDirectoryName(string str);
            void getStructParents(Instruction *I, vector<pair<Type *, int>> &typeList);
            /*** Determinator ***/
            long getMemberIndiceFromByte(StructType * STy, uint64_t byte);
            bool isStructEleAlloc(Instruction *);
            bool isStructEleFree(Instruction *);
            bool isStructFree(Instruction *);
            bool isOptimizedStructFree(Instruction *I);
            Type* getOptimizedStructFree(Instruction *I);
            Value* getStructFreedValue(Instruction * val, bool isUserDefCalled = false);
            Value* getCalledStructFreedValue(Instruction * val);
            bool isHeapValue(Value *);
            bool isFuncPointer(Type * t);
            bool isBidirectionalAlias(Value *V);
            GetElementPtrInst* getAllocStructEleInfo(Instruction *);
            GetElementPtrInst* getFreeStructEleInfo(Instruction *);
            GetElementPtrInst* getStoredStructEle(StoreInst * SI);
            GetElementPtrInst* getStoredStruct(StoreInst * SI);
            Type * getStructType(Instruction * val);
            Value * getFreedValue(Instruction * val);
            Value * getAllocatedValue(Instruction *I);
            /*** Support Methods ***/
            vector<long> getValueIndices(GetElementPtrInst * inst);
            GetElementPtrInst *getRootGEle(GetElementPtrInst *GEle);
            Type* extractResultElementType(GetElementPtrInst *GEle);
            void reversePropagateErrorBlockFreeInfo();
            /*** connector with struct manager***/
            bool isAuthorityChained(ParentList);
            /*** find icmp ***/
            ICmpInst* findAllocICmp(Instruction *I);
            /*** MethodMap ***/
            map<unsigned, void (ST_free::BaseAnalyzer::*)(Instruction*, BasicBlock&)> InstAnalysisMap;
        private:
            /*** private methods ***/
            void __recursiveReversePropagateErrorBlockFreeInfo(BasicBlock *B);
            /*** Managers and DataLayouts ***/
            FunctionManager identifier;
            LoopManager *loopmap;
            StructManager *stManage;
            const DataLayout* dat_layout;
            /*** Current Function/ Stacked Functions ***/
            FunctionInformation *FEle;
            stack<Function *> functionStack;
    };
}
