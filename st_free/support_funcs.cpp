#include "support_funcs.hpp"

using namespace ST_free;

namespace ST_free {
    /*** Iterate Use until Load Instruction ***/
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

    Value * getLoadeeValue(Value * val){
        if(LoadInst *inst = dyn_cast<LoadInst>(val)){
            // outs() << *(inst->getPointerOperand()) << "\n";
            return inst->getPointerOperand();
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

    void generateWarning(Instruction * Inst, string warn){
        if(const DebugLoc &Loc = Inst->getDebugLoc()){
            unsigned line = Loc.getLine();
            unsigned col = Loc.getCol();
            outs() << "\033[1;31m[ST_free]\033[0m ";
            // outs() << string(Loc->getDirectory()) << "/" << string(Loc->getFilename()) << ":" << line << ":" << col << ": ";
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
}
