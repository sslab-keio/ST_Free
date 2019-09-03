#include "support_funcs.hpp"

using namespace ST_free;

namespace ST_free {
    /*** Iterate Use until Load Instruction ***/
    LoadInst * find_load(Instruction * val){
        return find_load_recursively(val, 3);
    }

    static LoadInst * find_load_recursively(Instruction *val, int TTL) {
        if(isa<LoadInst>(val))
            return cast<LoadInst>(val);

        if(TTL >= 0){
            for(Use &U : val->operands()){
                if(Instruction * inst = dyn_cast<Instruction>(U)){
                    if(isa<LoadInst>(inst))
                        return cast<LoadInst>(inst);

                    if(LoadInst * res = find_load_recursively(inst, --TTL))
                        return res;
                }
            }
        }
        return NULL;
    }

    Value * getLoadeeValue(Value * val){
        if(LoadInst *inst = dyn_cast<LoadInst>(val)){
            // if(GetElementPtrInst *gele = dyn_cast<GetElementPtrInst>(inst->getPointerOperand()))
            //     return getLoadeeValue(gele->getPointerOperand());
            return inst->getPointerOperand();
        }
        return NULL;
    }
    /*** Retrieve Pointer Dereferance Type ***/
    Type * get_type(Value * val){
        Type * val_type;

        if(val == NULL){
            return NULL;
        }

        val_type = val->getType();
        while(val_type->isPointerTy()){
            val_type = (cast<PointerType>(val_type))->getElementType();
        }
        return val_type;
    }

    Type * get_type(Type * t){
        Type * val_type;

        if(t == NULL)
            return NULL;
        val_type = t;
        while(val_type->isPointerTy()){
            val_type = (cast<PointerType>(val_type))->getElementType();
        }
        return val_type;
    }
    void generateWarning(Instruction * Inst, string warn){
        if(const DebugLoc &Loc = Inst->getDebugLoc()){
            unsigned line = Loc.getLine();
            unsigned col = Loc.getCol();
            DEBUG_WITH_TYPE("st_free", outs() << "\033[1;34m[ST_free]\033[0m ");
            DEBUG_WITH_TYPE("st_free", outs() << string(Loc->getFilename()) << ":" << line << ":" << col << ": ");
            DEBUG_WITH_TYPE("st_free", outs() << warn << "\n");
        }
    }
    void generateWarning(string warn){
            DEBUG_WITH_TYPE("st_free", outs() << "\033[1;34m[ST_free]\033[0m: ");
            DEBUG_WITH_TYPE("st_free", outs() << warn << "\n");
    }

    void generateError(Instruction * Inst, string warn){
        if(const DebugLoc &Loc = Inst->getDebugLoc()){
            unsigned line = Loc.getLine();
            unsigned col = Loc.getCol();
            outs() << "\033[1;31m[ST_free]\033[0m ";
            outs() << string(Loc->getFilename()) << ":" << line << ":" << col << ": ";
            outs() << warn << "\n";
        }
    }

    Value * getArgAlloca(Value *arg){
        for(auto usr = arg->user_begin(); usr != arg->user_end();usr++){
             if(StoreInst * str_inst = dyn_cast<StoreInst>(*usr)){
                return str_inst->getOperand(1);
             }
        }
        return NULL;
    }

    long getValueIndices(GetElementPtrInst * inst){
        long indice = 0;
        for(auto idx_itr = inst->idx_begin(); idx_itr != inst->idx_end(); idx_itr++){
            if (idx_itr == inst->idx_begin())
                continue;
            if(ConstantInt * cint = dyn_cast<ConstantInt>(idx_itr->get())) 
                indice = cint->getSExtValue();
        }
        return indice;
    }
    User * getFirstUser(Value * v){
        for(User * usr : v->users())
            return usr;
    }
}
