#include "analyzer.hpp"
#include "determinator.hpp"
#include "support_funcs.hpp"

namespace ST_free{
    FuncIdentifier Analyzer::identifier;
    StatusList Analyzer::stat;
    void Analyzer::analyze(Function &F){
        if(identifier.isInMap(&F))
            return;

        identifier.setFunction(&F);
        for(auto args = F.arg_begin(); args != F.arg_end(); args++){
            Value * v = getArgAlloca(args);
            if(v != NULL){
                arg_list.push_back(v);
            }
        }

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
                                        } else if (isStructFree(val)) {
                                            this->checkStructElements(val);
                                        }
                                    }
                                }
                            }
                        } else {
                            this->analyzeDifferentFunc((Function &)(*(CI->getCalledFunction())));
                            vector<int> * statList = identifier.getStatusList(CI->getCalledFunction());
                            if(statList != NULL){
                                for(uint64_t i = 0; i < statList->size(); i++){
                                    if((*statList)[i] != NO_ALLOC){
                                        //TODO
                                        Value * val = CI->getOperand(i);
                                        if (Instruction * inst = dyn_cast<Instruction>(val)) {
                                            if (PointerType * ptr_ty = dyn_cast<PointerType>(inst->getType())) {
                                                LoadInst * load_inst = find_load(inst);
                                                Value * loadee = getLoadeeValue(load_inst);
                                                if(loadee != NULL && stat.isInList(loadee->getType(), val)){
                                                    stat.setStat(loadee->getType(), val, i, ((*statList)[i]));
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
        }
        return;
    }
    void Analyzer::analyzeDifferentFunc(Function &F){
        Analyzer called_function;
        called_function.analyze(F);
        return;
    }
    void Analyzer::checkStructElements(Instruction * val){
        int index = 0;
        LoadInst *load_inst = find_load(val);
        StructType * tgt_type = cast<StructType>(get_type(load_inst->getPointerOperand()));
        for(auto ele = tgt_type->element_begin(); ele != tgt_type->element_end(); ele++, index++){
            if((*ele)->isPointerTy()){
                generateWarning(load_inst, "Has pointer element");

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
}
