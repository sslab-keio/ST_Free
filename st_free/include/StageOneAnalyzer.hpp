#include "ST_free.hpp"
#include "analyzer.hpp"

namespace ST_free {
    class StageOneAnalyzer : public BaseAnalyzer {
        public:
            StageOneAnalyzer(Function *func, StructManager *stm, LoopManager *lmap) : BaseAnalyzer(func, stm, lmap){};
            void analyzeDifferentFunc(Function &F);
        protected:
            /*** Availability Analysis ***/
            void checkAvailability();
            /*** Instruction Analysis ***/
            void analyzeInstructions(BasicBlock &B);
            void analyzeAllocaInst(AllocaInst * AI, BasicBlock &B);
            void analyzeStoreInst(StoreInst * SI, BasicBlock &B);
            void analyzeCallInst(CallInst *CI, BasicBlock &B);
            void analyzeBranchInst(BranchInst * BI, BasicBlock &B);
    };
}
