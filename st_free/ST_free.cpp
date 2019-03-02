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
                            if (isAllocFunction(string(called_function->getName()))) {
                            // if (isAllocFunction(called_function)) {
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
                            } else if (isFreeFunction(string(called_function->getName()))) {
                            // } else if (isFreeFunction(called_function)) {
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

        // bool isAllocFunction(string name){
        //     auto itr = find(alloc_funcs.begin(), alloc_funcs.end(), name);
        //     if(itr != alloc_funcs.end()){
        //         return true;
        //     }
        //     return false;
        // }

        // bool isFreeFunction(string name){
        //     auto itr = find(free_funcs.begin(), free_funcs.end(), name);
        //     if(itr != free_funcs.end()){
        //         return true;
        //     }
        //     return false;
        // }

        // /*** Iterate Use until Load Instruction ***/
        // LoadInst * find_load(Instruction * val){
        //     if(isa<LoadInst>(val)){
        //         return cast<LoadInst>(val);
        //     }
        //     for(Use &U : val->operands()){
        //         if(Instruction * inst = dyn_cast<Instruction>(U)){
        //             if(isa<LoadInst>(inst)){
        //                 return cast<LoadInst>(inst);
        //             }
        //         }
        //     }
        //     return NULL;
        // }

        // bool isInList(Type *T, Value *V){
        //     if(st_tab.find(T) != st_tab.end()){
        //         if(st_tab[T].find(V) != st_tab[T].end()){
        //             return true;
        //         }
        //     }
        //     return false;
        // }

        // int getStat(Type *T, Value *V, uint64_t index){
        //     if(isInList(T, V)){
        //         for (auto ele = st_tab[T][V].begin();ele != st_tab[T][V].end(); ele++){
        //             if(ele->index == index)
        //                 return ele->status;
        //         }
        //     }
        //     return NO_ALLOC;
        // }

        // void setStat(Type *T, Value *V, uint64_t index, int stat){
        //     if(isInList(T, V)){
        //         for (auto ele = st_tab[T][V].begin();ele != st_tab[T][V].end(); ele++){
        //             if(ele->index == index){
        //                 ele->status = stat;
        //                 return;
        //             }
        //         }
        //     }
        // }

        // Value * getLoadeeValue(Value * val){
        //     if(LoadInst *inst = dyn_cast<LoadInst>(val)){
        //         // outs() << *(inst->getPointerOperand()) << "\n";
        //         return inst->getPointerOperand();
        //     }
        //     return NULL;
        // }

        // bool isInList(Type *T, Value *V, uint64_t index){
        //     vector<status_element> stat_list = st_tab[T][V];
        //     auto ele = find(stat_list.begin(), stat_list.end(), status_element(T, V, index));
        //     if(ele != stat_list.end())
        //         return true;
        //     return false;
        // }

        // bool isStructEleAlloc(Instruction * val){
        //     for (User *usr: val->users()){
        //         User * tmp_usr = usr;
        //         if(!isa<StoreInst>(usr)){
        //             for(User * neo_usr: usr->users()){
        //                 if(isa<StoreInst>(neo_usr)){
        //                     tmp_usr = neo_usr;
        //                     break;
        //                 }
        //             }
        //         }
        //         if(StoreInst * str_inst = dyn_cast<StoreInst>(tmp_usr)){
        //             Value * tgt_op = str_inst->getOperand(1);
        //             // outs() << string(tgt_op->getName()) << "\n";
        //             if(GetElementPtrInst * inst = dyn_cast<GetElementPtrInst>(tgt_op)){
        //                 status_element st_ele(inst->getSourceElementType(),
        //                         getLoadeeValue(inst->getPointerOperand()),
        //                         cast<ConstantInt>(inst->getOperand(2))->getZExtValue());
        //                 if (!isInList(inst->getSourceElementType(), getLoadeeValue(inst->getPointerOperand())) &&
        //                         !isInList(inst->getSourceElementType(), getLoadeeValue(inst->getPointerOperand()), cast<ConstantInt>(inst->getOperand(2))->getZExtValue())){
        //                     st_tab[inst->getSourceElementType()][getLoadeeValue(inst->getPointerOperand())].push_back(st_ele);
        //                 }
        //                 // outs() << *(inst->getSourceElementType()) << "\n";
        //                 // outs() << cast<ConstantInt>(inst->getOperand(2))->getZExtValue() << "\n";
        //                 return true;
        //             }
        //         }
        //     }
        //     return false;
        // }

        // bool isStructEleFree(Instruction * val){
        //     LoadInst * l_inst = find_load(val);
        //     for(Use &U : l_inst->operands()){
        //         if(GetElementPtrInst * inst = dyn_cast<GetElementPtrInst>(U)){
        //             status_element st_ele(inst->getSourceElementType(),
        //                     getLoadeeValue(inst->getPointerOperand()),
        //                     cast<ConstantInt>(inst->getOperand(2))->getZExtValue());
        //             if (isInList(inst->getSourceElementType(), getLoadeeValue(inst->getPointerOperand())) &&
        //                     isInList(inst->getSourceElementType(), getLoadeeValue(inst->getPointerOperand()), cast<ConstantInt>(inst->getOperand(2))->getZExtValue())){
        //                 generateWarning(l_inst, "Found Allocated Element Free");
        //                 setStat(st_ele.struct_type, st_ele.v, st_ele.index, FREED);
        //                 return true;
        //             }
        //         }
        //     }
        //     return false;
        // }

        // bool isStructFree(Instruction * val){
        //     LoadInst *load_inst = find_load(val);
        //     if (load_inst != NULL) {
        //         Type * tgt_type = get_type(load_inst->getPointerOperand());
        //         if (tgt_type != NULL && tgt_type->isStructTy()) {
        //             generateWarning(load_inst, "Found Struct");
        //             return true;
        //         }
        //     }
        //     return false;
        // }

        void checkStructElements(Instruction * val){
            int index = 0;
            LoadInst *load_inst = find_load(val);
            StructType * tgt_type = cast<StructType>(get_type(load_inst->getPointerOperand()));
            for(auto ele = tgt_type->element_begin(); ele != tgt_type->element_end(); ele++, index++){
                if((*ele)->isPointerTy()){
                    generateWarning(load_inst, "Has pointer element");
                    // status_element st_ele(tgt_type, load_inst->getPointerOperand(), index);

                    // if(stat.isInList(tgt_type, load_inst->getPointerOperand())){
                    //     vector<status_element> stat_itr = st_tab[tgt_type][load_inst->getPointerOperand()];
                    //     for (auto st_ele = stat_itr.begin();st_ele != stat_itr.end(); st_ele++){
                    //         if(st_ele->status != FREED){
                    //             generateWarning(load_inst, "Unfreed pointer element found !!");
                    //         }
                    //     }
                    // }
                }
            }
        }

        /*** Retrieve Pointer Dereferance Type ***/
        // Type * get_type(Value * val){
        //     Type * val_type;

        //     if(val_type == NULL){
        //         return NULL;
        //     }

        //     val_type = val->getType();
        //     while(val_type->isPointerTy()){
        //         val_type = (cast<PointerType>(val_type))->getElementType();
        //     }
        //     return val_type;
        // }

        // bool isHeapValue(Value *v){
        //     return true;
        // }

        // void generateWarning(Instruction * Inst, string warn){
        //     if(const DebugLoc &Loc = Inst->getDebugLoc()){
        //         unsigned line = Loc.getLine();
        //         unsigned col = Loc.getCol();
        //         outs() << "\033[1;31m[ST_free]\033[0m ";
        //         // outs() << string(Loc->getDirectory()) << "/" << string(Loc->getFilename()) << ":" << line << ":" << col << ": ";
        //         outs() << string(Loc->getFilename()) << ":" << line << ":" << col << ": ";
        //         outs() << warn << "\n";
        //     }
        // }
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

