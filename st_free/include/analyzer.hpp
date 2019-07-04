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
    class Analyzer {
        private:
            static FunctionManager identifier;
            LoopManager *loopmap;
            FunctionInformation *FEle;
            StructManager *stManage;
            /*** Availability Analysis ***/
            void checkAvailability();
            /*** Instruction Analysis ***/
            void analyzeInstructions(BasicBlock &B);
            void analyzeAllocaInst(AllocaInst * AI, BasicBlock &B);
            void analyzeStoreInst(StoreInst * SI, BasicBlock &B);
            void analyzeCallInst(CallInst *CI, BasicBlock &B);
            void analyzeBranchInst(BranchInst * BI, BasicBlock &B);
            bool isReturnFunc(Instruction *I);
            /*** add Value ***/
            void addFree(Value * V, CallInst *CI, BasicBlock *B, bool isAlias = false);
            void addAlloc(CallInst *CI, BasicBlock *B);
            void addLocalStruct(BasicBlock * B, Type * T, Value * V, Instruction * I, ParentList P);
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
            Analyzer(){
            }
            Analyzer(Function *func, StructManager *stm, LoopManager *lmap) {
                FEle = identifier.getElement(func);
                loopmap = lmap;
                stManage = stm;
                FEle->setLoopInfo(loopmap->get(func));
            }
            void analyze();
            void analyzeDifferentFunc(Function &);
    };
}
