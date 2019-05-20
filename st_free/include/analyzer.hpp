#include "ST_free.hpp"
#include "functionManager.hpp"
// #include "inter_analysis.hpp"
#include "statList.hpp"
#include "argList.hpp"
#include "BBWorklist.hpp"

namespace ST_free {
    class Analyzer {
        private:
            static FunctionManager identifier;
            static StatusList stat;
            FunctionInformation * FEle;
            void checkAvailability();
            void analyzeInstructions(BasicBlock &B);
            void checkStructElements(Instruction *);
            void addFree(Value * V, CallInst *CI, BasicBlock *B);
            void addAlloc(CallInst *CI, BasicBlock *B);
            bool isReturnFunc(Instruction *I);
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
