#include "BaseAnalyzer.hpp"
#include "ST_free.hpp"

namespace ST_free {
class StageTwoAnalyzer : public BaseAnalyzer {
 public:
  StageTwoAnalyzer(llvm::Function *func, StructManager *stm)
      : BaseAnalyzer(func, stm){};
  void analyzeDifferentFunc(llvm::Function &F);

 protected:
  /*** Availability Analysis ***/
  void checkAvailability();
  /*** Instruction Analysis ***/
  void analyzeInstructions(llvm::BasicBlock &B);
  void analyzeAllocaInst(llvm::AllocaInst *AI, llvm::BasicBlock &B);
  void analyzeStoreInst(llvm::StoreInst *SI, llvm::BasicBlock &B);
  void analyzeCallInst(llvm::CallInst *CI, llvm::BasicBlock &B);
  void analyzeBranchInst(llvm::BranchInst *BI, llvm::BasicBlock &B);
};
}  // namespace ST_free
