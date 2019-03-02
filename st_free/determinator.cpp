#include "determinator.hpp"
#include "support_funcs.hpp"

using namespace ST_free;

const vector<string> alloc_funcs = {"malloc", "kzalloc", "kmalloc", "zalloc", "vmalloc", "kcalloc"};
const vector<string> free_funcs = {"free", "kfree"};

namespace ST_free {
    bool isAllocFunction(string name){
        auto itr = find(alloc_funcs.begin(), alloc_funcs.end(), name);
        if(itr != alloc_funcs.end()){
            return true;
        }
        return false;
    }

    // bool isAllocFunction(Function *F){
    //     string name = F->getName();
    //     // if(name != NULL){
    //         auto itr = find(alloc_funcs.begin(), alloc_funcs.end(), name);
    //         if(itr != alloc_funcs.end()){
    //             return true;
    //         }
    //     // }
    //     return false;
    // }

    bool isFreeFunction(string name){
        auto itr = find(free_funcs.begin(), free_funcs.end(), name);
        if(itr != free_funcs.end()){
            return true;
        }
        return false;
    }

    // bool isFreeFunction(Function *F){
    //     string name = F->getName();
    //     // if(name != NULL){
    //         auto itr = find(free_funcs.begin(), free_funcs.end(), name);
    //         if(itr != free_funcs.end()){
    //             return true;
    //         }
    //     // }
    //     return false;
    // }

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
                    // status_element st_ele(inst->getSourceElementType(),
                    //         getLoadeeValue(inst->getPointerOperand()),
                    //         cast<ConstantInt>(inst->getOperand(2))->getZExtValue());
                    // if (!isInList(inst->getSourceElementType(), getLoadeeValue(inst->getPointerOperand())) &&
                    //         !isInList(inst->getSourceElementType(), getLoadeeValue(inst->getPointerOperand()), cast<ConstantInt>(inst->getOperand(2))->getZExtValue())){
                    //     st_tab[inst->getSourceElementType()][getLoadeeValue(inst->getPointerOperand())].push_back(st_ele);
                    // }
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
                // status_element st_ele(inst->getSourceElementType(),
                //         getLoadeeValue(inst->getPointerOperand()),
                //         cast<ConstantInt>(inst->getOperand(2))->getZExtValue());
                // if (isInList(inst->getSourceElementType(), getLoadeeValue(inst->getPointerOperand())) &&
                //         isInList(inst->getSourceElementType(), getLoadeeValue(inst->getPointerOperand()), cast<ConstantInt>(inst->getOperand(2))->getZExtValue())){
                //     generateWarning(l_inst, "Found Allocated Element Free");
                //     changeIndexStatus(st_ele.struct_type, st_ele.v, st_ele.index, FREED);
                    return true;
                // }
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
