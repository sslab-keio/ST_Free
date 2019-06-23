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
            LoopManager * loopmap;
            FunctionInformation * FEle;
            StructManager * stManage;
            void checkAvailability();
            void analyzeInstructions(BasicBlock &B);
            void addFree(Value * V, CallInst *CI, BasicBlock *B);
            void addAlloc(CallInst *CI, BasicBlock *B);
            bool isReturnFunc(Instruction *I);
            void copyArgStatus(Function &Func, CallInst *CI, BasicBlock &B);
            void addLocalStruct(BasicBlock * B, Type * T, Value * V, Instruction * I, ParentList P);
            void addLocalVariable(BasicBlock * B, Type * T, Value * V, Instruction * I, ParentList P);
            void addPointerLocalVariable(BasicBlock *B, Type * T, Value * V, Instruction * I, ParentList P);
            bool isCorrectlyBranched(BranchInst * BI);
            bool isLoopBlock(BasicBlock& B);
        public:
            Analyzer(){
            }
            // explicit Analyzer(Function *func) {
            //     FEle = identifier.getElement(func);
            // }

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
