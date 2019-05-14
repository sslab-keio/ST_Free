#include "ST_free.hpp"
#include "statList.hpp"
#include "determinator.hpp"
#include "support_funcs.hpp"
#include "inter_analysis.hpp"
#include "analyzer.hpp"

using namespace ST_free;

namespace{
    struct st_free : public ModulePass {
        static char ID;

        st_free() : ModulePass(ID){
        }

        /*** Main Modular ***/
        bool runOnModule(Module &M) override {
            for(Function &F: M){
                // outs() << F.getName() << "\n";
                Analyzer analyze(&F);
                analyze.analyze();
            }
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
