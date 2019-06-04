#include "ST_free.hpp"
#include "functionManager.hpp"
#include "statList.hpp"
#include "argList.hpp"
#include "BBWorklist.hpp"
#include "ValueInformation.hpp"

namespace ST_free {
    class Analyzer {
        private:
            static FunctionManager identifier;
            FunctionInformation * FEle;
            void checkAvailability();
            void analyzeInstructions(BasicBlock &B);
            void addFree(Value * V, CallInst *CI, BasicBlock *B);
            void addAlloc(CallInst *CI, BasicBlock *B);
            bool isReturnFunc(Instruction *I);
            void copyArgStatus(Function &Func, CallInst *CI, BasicBlock &B);
            void addLocalStruct(BasicBlock * B, Type * T, Value * V, Instruction * I, ParentList P);
            void addLocalVariable(BasicBlock * B, Type * T, Value * V, Instruction * I, ParentList P);
        public:
            Analyzer(){
            }
            explicit Analyzer(Function *func) {
                FEle = identifier.getElement(func);
            }
            void analyze();
            void analyzeDifferentFunc(Function &);
    };
}
