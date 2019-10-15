#include "ST_free.hpp"
#include "BaseAnalyzer.hpp"

namespace ST_free {
    class StageOneAnalyzer : public BaseAnalyzer {
        public:
            StageOneAnalyzer(Function *func, StructManager *stm, LoopManager *lmap, const DataLayout *dl) : BaseAnalyzer(func, stm, lmap, dl){};
            StageOneAnalyzer(StructManager *stm, LoopManager *lmap, const DataLayout *dl) : BaseAnalyzer(stm, lmap, dl){};
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
