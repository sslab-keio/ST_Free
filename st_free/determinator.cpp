#include "determinator.hpp"
#include "support_funcs.hpp"

using namespace ST_free;

const vector<string> alloc_funcs = {"malloc", "kzalloc", "kmalloc", "zalloc", "vmalloc", "kcalloc"};
const vector<string> free_funcs = {"free", "kfree"};

namespace ST_free {
    bool isAllocFunction(Function *F){
        string name = F->getName();
        auto itr = find(alloc_funcs.begin(), alloc_funcs.end(), name);
        if(itr != alloc_funcs.end()){
            return true;
        }
        return false;
    }

    bool isFreeFunction(Function *F){
        string name = F->getName();
        auto itr = find(free_funcs.begin(), free_funcs.end(), name);
        if(itr != free_funcs.end()){
            return true;
        }
        return false;
    }

    bool isStructEleAlloc(Instruction * val){
        for (User *usr: val->users()){
            User * tmp_usr = usr;
            if(!isa<StoreInst>(usr)){
                for(User * neo_usr: usr->users()){
                    if(isa<StoreInst>(neo_usr)){
                        tmp_usr = neo_usr;
                        break;
                    }
                }
            }
            if(StoreInst * str_inst = dyn_cast<StoreInst>(tmp_usr)){
                Value * tgt_op = str_inst->getOperand(1);
                if(GetElementPtrInst * inst = dyn_cast<GetElementPtrInst>(tgt_op)){
                    return true;
                }
            }
        }
        return false;
    }

    GetElementPtrInst * getAllocStructEleInfo(Instruction * val){
        for (User *usr: val->users()){
            User * tmp_usr = usr;
            if(!isa<StoreInst>(usr)){
                for(User * neo_usr: usr->users()){
                    if(isa<StoreInst>(neo_usr)){
                        tmp_usr = neo_usr;
                        break;
                    }
                }
            }
            if(StoreInst * str_inst = dyn_cast<StoreInst>(tmp_usr)){
                Value * tgt_op = str_inst->getOperand(1);
                if(GetElementPtrInst * inst = dyn_cast<GetElementPtrInst>(tgt_op)){
                    return inst;
                }
            }
        }
        return NULL;
    }

    bool isStructEleFree(Instruction * val){
        LoadInst * l_inst = find_load(val);
        for(Use &U : l_inst->operands()){
            if(GetElementPtrInst * inst = dyn_cast<GetElementPtrInst>(U)){
                return true;
            }
        }
        return false;
    }

    GetElementPtrInst* getFreeStructEleInfo(Instruction * val){
        LoadInst * l_inst = find_load(val);
        for(Use &U : l_inst->operands()){
            if(GetElementPtrInst * inst = dyn_cast<GetElementPtrInst>(U)){
                return inst;
            }
        }
        return NULL;
    }

    bool isStructFree(Instruction * val){
        LoadInst *load_inst = find_load(val);
        if (load_inst != NULL) {
            Type * tgt_type = get_type(load_inst->getPointerOperand());
            if (tgt_type != NULL && tgt_type->isStructTy()) {
                generateWarning(load_inst, "Found Struct");
                return true;
            }
        }
        return false;
    }

    bool isHeapValue(Value *v){
        return true;
    }
}
