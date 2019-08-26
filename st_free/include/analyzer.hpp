#pragma once
#include "ST_free.hpp"
#include "functionManager.hpp"
#include "statList.hpp"
#include "argList.hpp"
#include "BBWorklist.hpp"
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
            BaseAnalyzer(Function *func, StructManager *stm, LoopManager *lmap) {
                FEle = identifier.getElement(func);
                loopmap = lmap;
                stManage = stm;
                FEle->setLoopInfo(loopmap->get(func));
            }
            BaseAnalyzer(StructManager *stm, LoopManager *lmap){
                loopmap = lmap;
                stManage = stm;
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
            /*** Availability Analysis ***/
            virtual void checkAvailability();
            /*** Instruction Analysis ***/
            virtual void analyzeInstructions(BasicBlock &B);
            virtual void analyzeAllocaInst(AllocaInst * AI, BasicBlock &B);
            virtual void analyzeStoreInst(StoreInst * SI, BasicBlock &B);
            virtual void analyzeCallInst(CallInst *CI, BasicBlock &B);
            virtual void analyzeBranchInst(BranchInst * BI, BasicBlock &B);
            bool isReturnFunc(Instruction *I);
            /*** add Value ***/
            void addFree(Value * V, CallInst *CI, BasicBlock *B, bool isAlias = false);
            void addAlloc(CallInst *CI, BasicBlock *B);
            void addLocalVariable(BasicBlock * B, Type * T, Value * V, Instruction * I, ParentList P);
            void addPointerLocalVariable(BasicBlock *B, Type * T, Value * V, Instruction * I, ParentList P);
            /*** Argument Status ***/
            void copyArgStatus(Function &Func, CallInst *CI, BasicBlock &B);
            /*** Branch Instruction(Correctly Branched) ***/
            bool isCorrectlyBranched(BranchInst * BI);
            /*** Store Instruction related funtions ***/
            bool isStoreToStructMember(StoreInst * SI);
            bool isStoreFromStructMember(StoreInst * SI);
            bool isStoreToStruct(StoreInst *SI);
            bool isStoreFromStruct(StoreInst *SI);
            UniqueKey decodeGEPInst(GetElementPtrInst *GEle);
            vector<string> decodeDirectoryName(string str);
            void getStructParents(Instruction *I, vector<pair<Type *, int>> &typeList);
        private:
            FunctionManager identifier;
            LoopManager *loopmap;
            StructManager *stManage;
            FunctionInformation *FEle;
            stack<Function *> functionStack;
    };
}
