#include "ST_free.hpp"
#include "statList.hpp"
#include "determinator.hpp"
#include "support_funcs.hpp"

using namespace ST_free;

namespace{
    struct st_free : public FunctionPass {
        static char ID;
        StatusList stat;

        st_free() : FunctionPass(ID){
        }

        /*** Main Moduler ***/
        bool runOnFunction(Function &F) override {
            for (BasicBlock &B: F) {
                for (Instruction &I : B) {
                    if (auto* CI = dyn_cast<CallInst>(&I)) {
                        if (Function* called_function = CI->getCalledFunction()) {
                            if (isAllocFunction(called_function)) {
                                generateWarning(CI, "Found Malloc");
                                if(isStructEleAlloc(CI)){
                                    GetElementPtrInst *inst = getAllocStructEleInfo(CI);
                                    stat.setStat(
                                        inst->getSourceElementType(),
                                        getLoadeeValue(inst->getPointerOperand()),
                                        cast<ConstantInt>(inst->getOperand(2))->getZExtValue(),
                                        ALLOCATED
                                    );
                                    generateWarning(CI, "Struct element malloc");
                                }
                            } else if (isFreeFunction(called_function)) {
                                for (auto args = CI->arg_begin(); args != CI->arg_end();args++) {
                                    if (Instruction * val = dyn_cast<Instruction>(* args)) {
                                        if (PointerType * ptr_ty = dyn_cast<PointerType>(val->getType())) {
                                            if(isStructEleFree(val)){
                                                GetElementPtrInst * inst = getFreeStructEleInfo(CI);
                                                stat.setStat(
                                                    inst->getSourceElementType(),
                                                    getLoadeeValue(inst->getPointerOperand()),
                                                    cast<ConstantInt>(inst->getOperand(2))->getZExtValue(),
                                                    FREED
                                                );
                                                generateWarning(val, "Struct element free");
                                            } else if (isStructFree(val)){
                                                checkStructElements(val);
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

        void checkStructElements(Instruction * val){
            int index = 0;
            LoadInst *load_inst = find_load(val);
            StructType * tgt_type = cast<StructType>(get_type(load_inst->getPointerOperand()));
            for(auto ele = tgt_type->element_begin(); ele != tgt_type->element_end(); ele++, index++){
                if((*ele)->isPointerTy()){
                    generateWarning(load_inst, "Has pointer element");
                    // status_element st_ele(tgt_type, load_inst->getPointerOperand(), index);

                    if(stat.isInList(tgt_type, load_inst->getPointerOperand())){
                        vector<int> * itr_list = stat.getList(tgt_type, load_inst->getPointerOperand());
                        for(auto ele = itr_list->begin(); ele != itr_list->end(); ele++){
                            if(*ele == ALLOCATED){
                                generateWarning(load_inst, "Unfreed pointer element found !!");
                            }
                        }
                    }
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

