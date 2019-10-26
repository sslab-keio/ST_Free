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
            }
            BaseAnalyzer(StructManager *stm, LoopManager *lmap, const DataLayout *dl){
                loopmap = lmap;
                stManage = stm;
                dat_layout = dl;
            }
            void analyze(Function &F);
            void analyzeDifferentFunc(Function &);
        protected:
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
            virtual void analyzeAllocaInst(AllocaInst * AI, BasicBlock &B);
            virtual void analyzeStoreInst(StoreInst * SI, BasicBlock &B);
            virtual void analyzeCallInst(CallInst *CI, BasicBlock &B);
            virtual void analyzeBranchInst(BranchInst * BI, BasicBlock &B);
            virtual void analyzeBitCastInst(BitCastInst *BCI, BasicBlock &B);
            bool isReturnFunc(Instruction *I);
            /*** add Value ***/
            void addFree(Value * V, CallInst *CI, BasicBlock *B, bool isAlias = false, ParentList additionalParents = ParentList());
            void addAlloc(CallInst *CI, BasicBlock *B);
            void addLocalVariable(BasicBlock * B, Type * T, Value * V, Instruction * I, ParentList P);
            void addPointerLocalVariable(BasicBlock *B, Type * T, Value * V, Instruction * I, ParentList P);
            /*** Argument Status ***/
            void copyArgStatus(Function &Func, CallInst *CI, BasicBlock &B);
            void copyArgStatusRecursively(Function &Func, CallInst *CI, BasicBlock &B, Value* arg, ArgStatus *ArgStat, int ind, ParentList plist, bool isFirst = false);
            /*** Branch Instruction(Correctly Branched) ***/
            bool isCorrectlyBranched(BranchInst * BI);
            /*** Store Instruction related funtions ***/
            bool isStoreToStructMember(StoreInst * SI);
            bool isStoreFromStructMember(StoreInst * SI);
            bool isStoreToStruct(StoreInst *SI);
            bool isStoreFromStruct(StoreInst *SI);
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
            Value* getStructFreedValue(Instruction * val);
            bool isHeapValue(Value *);
            bool isFuncPointer(Type * t);
            GetElementPtrInst* getAllocStructEleInfo(Instruction *);
            GetElementPtrInst* getFreeStructEleInfo(Instruction *);
            GetElementPtrInst*  getStoredStructEle(StoreInst * SI);
            GetElementPtrInst*  getStoredStruct(StoreInst * SI);
            Type * getStructType(Instruction * val);
            Value * getFreedValue(Instruction * val);
            Value * getAllocatedValue(Instruction *I);
            /*** Support Methods ***/
            vector<long> getValueIndices(GetElementPtrInst * inst);
            GetElementPtrInst *getRootGEle(GetElementPtrInst *GEle);
            Type* extractResultElementType(GetElementPtrInst *GEle);
        private:
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
