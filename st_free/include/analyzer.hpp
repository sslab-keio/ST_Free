#include "ST_free.hpp"
#include "functionManager.hpp"
#include "statList.hpp"
#include "argList.hpp"
#include "BBWorklist.hpp"

namespace ST_free {
    class Analyzer {
        private:
            static FunctionManager identifier;
            FunctionInformation * FEle;
            void checkAvailability();
            void analyzeInstructions(BasicBlock &B);
            // void checkStructElements(Instruction *);
            void addFree(Value * V, CallInst *CI, BasicBlock *B);
            void addAlloc(CallInst *CI, BasicBlock *B);
            bool isReturnFunc(Instruction *I);
            void copyArgStatus(Function &Func, CallInst *CI, BasicBlock &B);
            void addLocalStruct(Type * T, Value *V, Instruction *I, ParentList P);
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
