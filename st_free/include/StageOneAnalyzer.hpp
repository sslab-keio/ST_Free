#include "BaseAnalyzer.hpp"
#include "ST_free.hpp"

namespace ST_free {
class StageOneAnalyzer : public BaseAnalyzer {
 public:
  StageOneAnalyzer(Function *func, StructManager *stm, LoopManager *lmap,
                   const DataLayout *dl)
      : BaseAnalyzer(func, stm, lmap, dl){};
  StageOneAnalyzer(StructManager *stm, LoopManager *lmap, const DataLayout *dl)
      : BaseAnalyzer(stm, lmap, dl){};

 protected:
  /*** Availability Analysis ***/
  void checkAvailability();
  /*** Instruction Analysis ***/
  void analyzeInstructions(BasicBlock &B);
  void analyzeAllocaInst(Instruction *AI, BasicBlock &B);
  void analyzeStoreInst(Instruction *SI, BasicBlock &B);
  void analyzeCallInst(Instruction *CI, BasicBlock &B);
  void analyzeBranchInst(Instruction *BI, BasicBlock &B);
  void analyzeGetElementPtrInst(Instruction *GEleI, BasicBlock &B);
};
}  // namespace ST_free
