#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DerivedTypes.h"

// include STL
#include <vector>
#include <map>
#include <queue>
#include <algorithm>

using namespace llvm;
using namespace std;

namespace{
    struct st_free : public FunctionPass {
        static char ID;

        st_free() : FunctionPass(ID){
        }

        /*** moduler ***/
        bool runOnFunction(Function &F) override {
            for (BasicBlock &B: F) {
                // outs() << B << "\n";
                for (Instruction &I : B) {
                    if (auto* CI = dyn_cast<CallInst>(&I)) {
                        if (Function* called_function = CI->getCalledFunction()){
                            if (string(called_function->getName()) == "free"){
                                for(auto args = CI->arg_begin(); args != CI->arg_end();args++){
                                    if(Instruction * val = dyn_cast<Instruction>(* args)){
                                        if(PointerType * ptr_ty = dyn_cast<PointerType>(val->getType())){
                                            Value *gep_inst = find_GEP();
                                            if(gep_inst != NULL){
                                                // TODO
                                            }
                                            // while(1){
                                            //     bool found = false;
                                            //     for(Use &U : val->operand()){
                                            //         outs() << *U << "\n";
                                            //         if(isa<GetElementPtrInst>(U)){
                                            //             found = true;
                                            //             break;
                                            //         }
                                            //     }
                                            //     if(found){
                                            //         outs() << "found!\n";
                                            //         break;
                                            //     }
                                            // }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
           return false;
        }
        Value * find_GEP(){
            return NULL;
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
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSTFreePass);

