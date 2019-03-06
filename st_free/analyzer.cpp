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
        args.setArgs(F);
        
        /*** BasicBlock Interator ***/
        for (BasicBlock &B: F) {
            /*** Instruction Iterator ***/
            for (Instruction &I : B) {
                /*** is Call Instruction ***/
                if (auto* CI = dyn_cast<CallInst>(&I)) {
                    /*** get Called Function ***/
                    if (Function* called_function = CI->getCalledFunction()) {
                        /*** is Allocation Function ***/
                        if (isAllocFunction(called_function)) {
                            generateWarning(CI, "Found Malloc");
                            /*** is Struct Element Allocation ***/
                            if(isStructEleAlloc(CI)){
                                /*** add status ***/
                                GetElementPtrInst *inst = getAllocStructEleInfo(CI);
                                stat.setStat(
                                    inst->getSourceElementType(),
                                    getLoadeeValue(inst->getPointerOperand()),
                                    cast<ConstantInt>(inst->getOperand(2))->getZExtValue(),
                                    ALLOCATED
                                );

                                /*** Is arg value ***/
                                if(args.isInList(getLoadeeValue(inst->getPointerOperand()))){
                                    identifier.setFunctionStatus(
                                        &F,
                                        args.getOperandNum(getLoadeeValue(inst->getPointerOperand())),
                                        ALLOCATED
                                    );
                                }
                                generateWarning(CI, "Struct element malloc");
                            }
                        } else if (isFreeFunction(called_function)) {
                            for (auto arguments = CI->arg_begin(); arguments != CI->arg_end();arguments++) {
                                if (Instruction * val = dyn_cast<Instruction>(* arguments)) {
                                    if (PointerType * ptr_ty = dyn_cast<PointerType>(val->getType())) {
                                        if(isStructEleFree(val)){
                                            GetElementPtrInst * inst = getFreeStructEleInfo(CI);
                                            stat.setStat(
                                                inst->getSourceElementType(),
                                                getLoadeeValue(inst->getPointerOperand()),
                                                cast<ConstantInt>(inst->getOperand(2))->getZExtValue(),
                                                FREED
                                            );
                                            // outs() << "Free " << *getLoadeeValue(inst->getPointerOperand()) << "\n";
                                            if(args.isInList(getLoadeeValue(inst->getPointerOperand()))) {
                                                identifier.setFunctionStatus(
                                                    &F,
                                                    args.getOperandNum(getLoadeeValue(inst->getPointerOperand())),
                                                    FREED
                                                );
                                            }
                                            generateWarning(val, "Struct element free");
                                        } else if (isStructFree(val)) {
                                            Value * loaded_value = getStructFreedValue(val);
                                            if(loaded_value != NULL){
                                                if(args.isInList(loaded_value)){
                                                    identifier.setFunctionStatus(
                                                        &F,
                                                        args.getOperandNum(loaded_value),
                                                        FREED
                                                    );
                                                }
                                            }
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
                                    if((*statList)[i] == ALLOCATED){
                                        // TODO
                                        // do allocation
                                    } else if((*statList)[i] == FREED){
                                        //TODO
                                        Value * val = CI->getOperand(i);
                                        if (Instruction * inst = dyn_cast<Instruction>(val)) {
                                            if (PointerType * ptr_ty = dyn_cast<PointerType>(inst->getType())) {
                                                if(isStructEleFree(inst)){
                                                    GetElementPtrInst * inst = getFreeStructEleInfo(CI);
                                                    stat.setStat(
                                                        inst->getSourceElementType(),
                                                        getLoadeeValue(inst->getPointerOperand()),
                                                        cast<ConstantInt>(inst->getOperand(2))->getZExtValue(),
                                                        FREED
                                                    );
                                                    // outs() << "Free " << *getLoadeeValue(inst->getPointerOperand()) << "\n";
                                                    if(args.isInList(getLoadeeValue(inst->getPointerOperand()))) {
                                                        identifier.setFunctionStatus(
                                                            &F,
                                                            args.getOperandNum(getLoadeeValue(inst->getPointerOperand())),
                                                            FREED
                                                        );
                                                    }
                                                    generateWarning(inst, "Struct element free");
                                                } else if (isStructFree(inst)) {
                                                    Value * loaded_value = getStructFreedValue(inst);
                                                    if(loaded_value != NULL){
                                                        if(args.isInList(loaded_value)){
                                                            identifier.setFunctionStatus(
                                                                &F,
                                                                args.getOperandNum(loaded_value),
                                                                FREED
                                                            );
                                                        }
                                                    }
                                                    this->checkStructElements(inst);
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
        Analyzer called_function(&F);
        if(!identifier.isInMap(&F))
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
