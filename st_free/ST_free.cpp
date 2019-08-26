#include "include/ST_free.hpp"
#include "include/statList.hpp"
#include "include/determinator.hpp"
#include "include/support_funcs.hpp"
#include "include/functionManager.hpp"
#include "include/StructInformation.hpp"
#include "include/LoopManager.hpp"

#include "include/analyzer.hpp"
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
            AU.addRequired<DominatorTreeWrapperPass>();
        }

        /*** Main Modular ***/
        bool runOnModule(Module &M) override {
            /*** Collect Struct Information ***/
            StructManager* StManage = new StructManager(M.getIdentifiedStructTypes());
            LoopManager* loopmap = new LoopManager();

            // StManage->addGlobalVariableInitInfo(M);
#ifdef STAGE_ONE
            StageOneAnalyzer analyze(StManage, loopmap);
#else
            BaseAnalyzer analyze(StManage, loopmap);
#endif

            /*** Generate LoopInformation ***/
            for(Function &F: M) {
                if(!(F.isDeclaration())) {
                    loopmap->add(&F, &(getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo()));
                }
            }

            int i = 0;
            /*** Main analysis module ***/
            for(Function &F: M) {
                outs() << i++ << "/" << M.size() << "\n";
                if(!(F.isDeclaration()))
                    analyze.analyze(F);
            }

            /*** Main Warning Generator ***/
            StManage->BuildCandidateCount();
            // StManage->print();
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
