#include "ST_free.hpp"
#include "inter_analysis.hpp"
#include "statList.hpp"
#include "argList.hpp"
#include "BBWorklist.hpp"

namespace ST_free {
    class Analyzer {
        private:
            static FunctionManager identifier;
            static StatusList stat;
            Function * Funcs;
            FuncElement * FEle;
        public:
            Analyzer(){
                FEle = NULL;
            }

            explicit Analyzer(Function *func) {
                FEle = identifier.getElement(func);
                Funcs = func;
            }
            void analyze();
            void analyze(Function * F);
            void analyzeDifferentFunc(Function &);
            void checkStructElements(Instruction *);
            void checkAndMarkFree(Value * V, CallInst *CI);
            void checkAndMarkAlloc(CallInst *CI);
            bool isReturnFunc(Instruction *I);
    };
}
