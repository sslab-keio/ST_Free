#include "include/ST_free.hpp"

#include "include/BaseAnalyzer.hpp"
#include "include/FunctionManager.hpp"
#include "include/StageOneAnalyzer.hpp"
#include "include/StructInformation.hpp"
#include "include/determinator.hpp"
#include "include/statList.hpp"
#include "include/support_funcs.hpp"

using namespace ST_free;

namespace {
struct st_free : public llvm::ModulePass {
  static char ID;

  st_free() : ModulePass(ID) {}

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
  }

  /*** Main Modular ***/
  bool runOnModule(llvm::Module &M) override {
    /*** Collect Struct Information ***/
    StructManager *StManage = new StructManager(M.getIdentifiedStructTypes());

    // StManage->addGlobalVariableInitInfo(M);
#if defined(STAGE_ONE) || defined(STAGE_PRIMITIVE)
    StageOneAnalyzer *analyze =
        new StageOneAnalyzer(StManage, &M.getDataLayout());
#elif defined(STAGE_TWO)
    // StageTwoAnalyzer* analyze = new StageTwoAnalyzer(StManage, 
    // &M.getDataLayout());
#else
    BaseAnalyzer *analyze =
        new BaseAnalyzer(StManage, &M.getDataLayout());
#endif

    /*** Additional analysis for Checking force casts ***/
    for (llvm::Function &F : M) {
      if (!(F.isDeclaration())) analyze->analyzeAdditionalUnknowns(F);
    }

    /*** Main analysis module ***/
    for (llvm::Function &F : M) {
      if (!(F.isDeclaration())) analyze->analyze(F);
    }

    /*** Main Warning Generator ***/
    StManage->BuildCandidateCount();
    StManage->print();
    StManage->checkCorrectness();
    return false;
  }
};  // end of struct
}  // end of anonymous namespace

char st_free::ID = 0;

static llvm::RegisterPass<st_free> X("st_free", "struct free checker",
                               false /* Only looks at CFG */,
                               false /* Analysis Pass */);

static void registerSTFreePass(const llvm::PassManagerBuilder &,
                               llvm::legacy::PassManagerBase &PM) {
  PM.add(new st_free());
}
// static RegisterStandardPasses
//   RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
//                  registerSTFreePass);
static llvm::RegisterStandardPasses RegisterMyPass(
    llvm::PassManagerBuilder::EP_ModuleOptimizerEarly, registerSTFreePass);

static llvm::RegisterStandardPasses RegisterMyPass1(
    llvm::PassManagerBuilder::EP_EnabledOnOptLevel0, registerSTFreePass);
