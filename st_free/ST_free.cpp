#include "include/ST_free.hpp"
#include "include/statList.hpp"
#include "include/determinator.hpp"
#include "include/support_funcs.hpp"
#include "include/FunctionManager.hpp"
#include "include/StructInformation.hpp"
#include "include/LoopManager.hpp"
#include "include/BaseAnalyzer.hpp"
#include "include/StageOneAnalyzer.hpp"

using namespace ST_free;

namespace{
    struct st_free : public ModulePass {
        static char ID;

        st_free() : ModulePass(ID){
        }

        virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
            AU.setPreservesAll();
            AU.addRequired<LoopInfoWrapperPass>();
        }

        /*** Main Modular ***/
        bool runOnModule(Module &M) override {
            /*** Collect Struct Information ***/
            StructManager* StManage = new StructManager(M.getIdentifiedStructTypes()); LoopManager* loopmap = new LoopManager();

            // StManage->addGlobalVariableInitInfo(M);
#if defined(STAGE_ONE) || defined(STAGE_PRIMITIVE)
            StageOneAnalyzer* analyze = new StageOneAnalyzer(StManage, loopmap, &M.getDataLayout());
#else
            BaseAnalyzer* analyze = new BaseAnalyzer(StManage, loopmap, &M.getDataLayout());
#endif

            /*** Generate LoopInformation ***/
            for(Function &F: M) {
                if(!(F.isDeclaration()))
                    loopmap->add(&F, &(getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo()));
            }

            /*** Additional analysis for Checking force casts ***/
            for(Function &F: M) {
                if(!(F.isDeclaration()))
                    analyze->analyzeAdditionalUnknowns(F);
            }

            /*** Main analysis module ***/
            for(Function &F: M) {
                if(!(F.isDeclaration()))
                    analyze->analyze(F);
            }

            /*** Main Warning Generator ***/
            StManage->BuildCandidateCount();
            StManage->print();
            StManage->checkCorrectness();
            return false;
        }
    }; // end of struct
}  // end of anonymous namespace

char st_free::ID = 0;

static RegisterPass<st_free> X("st_free", "struct free checker",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);

static void registerSTFreePass(const PassManagerBuilder &, legacy::PassManagerBase &PM) {
    PM.add(new st_free());
}
// static RegisterStandardPasses
//   RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
//                  registerSTFreePass);
static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_ModuleOptimizerEarly,
                                                registerSTFreePass);

static RegisterStandardPasses RegisterMyPass1(PassManagerBuilder::EP_EnabledOnOptLevel0,
                                               registerSTFreePass);
