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
                    // if(isa<BitCastInst>(inst))
                    //     generateError(inst, "Found bit cast");

                    if (LoadInst * res = find_load_recursively(inst, --TTL))
                        return res;
                }
            }
        }
        return NULL;
    }

    Value * getLoadeeValue(Value* val){
        Value *v = val;
        while (isa<LoadInst>(v) || isa<GetElementPtrInst>(v)) {
            if(auto inst = dyn_cast<LoadInst>(v)){
                v = inst->getPointerOperand();
            } else if(auto inst = dyn_cast<GetElementPtrInst>(v)){
                v = inst->getPointerOperand();
            }
        }

        return v;
    }
    /*** Retrieve Pointer Dereferance Type ***/
    Type * get_type(Value * val){
        Type * val_type = NULL;

        if(val == NULL)
            return NULL;

        if (auto allocaInst = dyn_cast<AllocaInst>(val)) {
            val_type = allocaInst->getAllocatedType();
            if (val_type->isPointerTy())
                val_type = (cast<PointerType>(val_type))->getElementType();
        } else if(auto GEleInst = dyn_cast<GetElementPtrInst>(val)) {
            val_type = GEleInst->getSourceElementType();
            if (val_type->isPointerTy())
                val_type = (cast<PointerType>(val_type))->getElementType();
        }

        return val_type;
    }

    Type* get_type(Type* t) {
        Type* val_type = NULL;

        if(t == NULL)
            return NULL;

        val_type = t;
        if (val_type->isPointerTy())
            val_type = (cast<PointerType>(val_type))->getElementType();
        return val_type;
    }

    void generateWarning(Instruction * Inst, string warn){
        if(const DebugLoc &Loc = Inst->getDebugLoc()){
            unsigned line = Loc.getLine();
            unsigned col = Loc.getCol();
            DEBUG_WITH_TYPE("st_free", outs() << "[WARNING] ");
            DEBUG_WITH_TYPE("st_free", outs() << string(Loc->getFilename()) << ":" << line << ":" << col << ": ");
            DEBUG_WITH_TYPE("st_free", outs() << warn << "\n");
        }
    }

    void generateWarning(Instruction * Inst, Value *val){
        if(const DebugLoc &Loc = Inst->getDebugLoc()){
            unsigned line = Loc.getLine();
            unsigned col = Loc.getCol();
            outs() << "[WARNING] ";
            outs() << string(Loc->getFilename()) << ":" << line << ":" << col << ": ";
            outs() << *val << "\n";
        }
    }

    void generateWarning(string warn) {
            DEBUG_WITH_TYPE("st_free", outs() << "[WARNING] ");
            DEBUG_WITH_TYPE("st_free", outs() << warn << "\n");
    }

    void generateError(Instruction * Inst, string warn){
        if(const DebugLoc &Loc = Inst->getDebugLoc()){
            unsigned line = Loc.getLine();
            unsigned col = Loc.getCol();
            outs() << "[ERROR] ";
            outs() << string(Loc->getFilename()) << ":" << line << ":" << col << ": ";
            outs() << warn << "\n";
        }
    }

    Value * getArgAlloca(Value *arg) {
        for(auto usr = arg->user_begin(); usr != arg->user_end();usr++){
             if(StoreInst * str_inst = dyn_cast<StoreInst>(*usr)){
                return str_inst->getOperand(1);
             }
        }
        return NULL;
    }


}
