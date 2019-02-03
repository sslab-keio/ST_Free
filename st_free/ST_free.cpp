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
                for (Instruction &I : B) {
                    if (auto* CI = dyn_cast<CallInst>(&I)) {
                        if (Function* called_function = CI->getCalledFunction()){
                            if (string(called_function->getName()) == "free"){
                                for(auto args = CI->arg_begin(); args != CI->arg_end();args++){
                                    if(Instruction * val = dyn_cast<Instruction>(* args)){
                                        if(PointerType * ptr_ty = dyn_cast<PointerType>(val->getType())){
                                            LoadInst *load_inst = find_load(val);
                                            if(load_inst != NULL){
                                                outs() <<  *(load_inst->getPointerOperand()) << "\n";
                                                Type * tgt_type = get_type(load_inst->getPointerOperand());
                                                if(tgt_type != NULL && tgt_type->isStructTy()){
                                                    outs() << "Found Struct\n";
                                                    check_struct(cast<StructType>(tgt_type));
                                                }
                                            }
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

        LoadInst * find_load(Instruction * val){
            if(isa<LoadInst>(val)){
                return cast<LoadInst>(val);
            }
            for(Use &U : val->operands()){
                if(Instruction * inst = dyn_cast<Instruction>(U)){
                    if(isa<LoadInst>(inst)){
                        return cast<LoadInst>(inst);
                    }
                }
            }
            return NULL;
        }
        /*** Retrieve Pointer Dereferance Type ***/
        Type * get_type(Value * val){
            Type * val_type;

            if(val_type == NULL){
                return NULL;
            }

            val_type = val->getType();
            while(val_type->isPointerTy()){
                val_type = (cast<PointerType>(val_type))->getElementType();
            }
            return val_type;
        }

        void check_struct(StructType *st_type){
            for(auto ele = st_type->element_begin(); ele != st_type->element_end(); ele++){
                outs() << **ele << "\n";
                if((*ele)->isPointerTy()){
                    outs() << "is Pointer\n";
                }
            }
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

