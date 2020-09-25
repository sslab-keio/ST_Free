#include "BaseAnalyzer.hpp"
#include "ST_free.hpp"

namespace ST_free {
class StageOneAnalyzer : public BaseAnalyzer {
 public:
  StageOneAnalyzer(llvm::Function *func, StructManager *stm, const llvm::DataLayout *dl)
      : BaseAnalyzer(func, stm, dl){};
  StageOneAnalyzer(StructManager *stm, const llvm::DataLayout *dl)
      : BaseAnalyzer(stm, dl){};

 protected:
  /*** Availability Analysis ***/
  void checkAvailability();
  /*** Instruction Analysis ***/
  void analyzeInstructions(llvm::BasicBlock &B);
  void analyzeAllocaInst(llvm::Instruction *AI, llvm::BasicBlock &B);
  void analyzeStoreInst(llvm::Instruction *SI, llvm::BasicBlock &B);
  void analyzeCallInst(llvm::Instruction *CI, llvm::BasicBlock &B);
  void analyzeBranchInst(llvm::Instruction *BI, llvm::BasicBlock &B);
  void analyzeSwitchInst(llvm::Instruction *SwI, llvm::BasicBlock &B);
  void analyzeGetElementPtrInst(llvm::Instruction *GEleI, llvm::BasicBlock &B);
};
}  // namespace ST_free
