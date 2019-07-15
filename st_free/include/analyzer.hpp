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

namespace ST_free {
    class BaseAnalyzer {
        private:
            static FunctionManager identifier;
            LoopManager *loopmap;
            FunctionInformation *FEle;
            StructManager *stManage;
        protected:
            /*** getter/setter ***/
            FunctionManager* getFunctionManager(){return &identifier;};
            LoopManager* getLoopManager(){return loopmap;};
            FunctionInformation* getFunctionInformation(){return FEle;};
            void setFunctionInformation(FunctionInformation * FInfo){FEle = FInfo;};
            StructManager* getStructManager(){return stManage;};
            void setStructManager(StructManager *stManager){stManage = stManager;};
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
            uniqueKey decodeGEPInst(GetElementPtrInst *GEle);
        public:
            BaseAnalyzer(){
            }
            BaseAnalyzer(Function *func, StructManager *stm, LoopManager *lmap) {
                FEle = identifier.getElement(func);
                loopmap = lmap;
                stManage = stm;
                FEle->setLoopInfo(loopmap->get(func));
            }
            void analyze();
            void analyzeDifferentFunc(Function &);
    };
}
