#include "analyzer.hpp"
#include "determinator.hpp"
#include "support_funcs.hpp"

namespace ST_free{
    FuncIdentifier Analyzer::identifier;
    StatusList Analyzer::stat;

    void Analyzer::analyze(){
        Function & F = (Function &)(*Funcs);

        if(identifier.exists(&F))
            return;

        identifier.initFuncStat(&F);
        args.setArgs(F);

        /*** BasicBlock Interator ***/
        for (BasicBlock &B: F) {
            // TODO: Check each prreceeder and add..
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
                            this->checkAndMarkAlloc(CI);
                        } else if (isFreeFunction(called_function)) {
                            for (auto arguments = CI->arg_begin(); arguments != CI->arg_end();arguments++) {
                                this->checkAndMarkFree(cast<Value>(arguments), CI);
                            }
                        } else {
                            this->analyzeDifferentFunc((Function &)(*called_function));
                            for(struct FuncElement ele :identifier.getArgStatList(called_function)){
                                if(ele.isArgAllocated()) {
                                    this->checkAndMarkAlloc(CI);
                                } else if(ele.isArgFreed()) {
                                    Value * val = CI->getOperand(ele.getArgNum());
                                    this->checkAndMarkFree(val, CI);
                                }
                            }
                        }
                    }
                }
            }
            // if(B.getSingleSuccessor()){
            // }
        }
        return;
    }

    void Analyzer::analyzeDifferentFunc(Function &F){
        Analyzer called_function(&F);
        if(!identifier.exists(&F))
            called_function.analyze();
        return;
    }

    void Analyzer::checkStructElements(Instruction * val){
        int index = 0;
        LoadInst *load_inst = find_load(val);
        StructType * tgt_type = cast<StructType>(get_type(load_inst->getPointerOperand()));
        for(auto ele = tgt_type->element_begin(); ele != tgt_type->element_end(); ele++, index++){
            if((*ele)->isPointerTy()){
                generateWarning(load_inst, "Has pointer element");

                if(stat.exists(tgt_type, load_inst->getPointerOperand())){
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

    void Analyzer::checkAndMarkFree(Value * V, CallInst *CI) {
        if (Instruction * val = dyn_cast<Instruction>(V)) {
            if (PointerType * ptr_ty = dyn_cast<PointerType>(val->getType())) {
                if(isStructEleFree(val)) {
                    GetElementPtrInst * inst = getFreeStructEleInfo(CI);
                    stat.setStat(
                        inst->getSourceElementType(),
                        getLoadeeValue(inst->getPointerOperand()),
                        cast<ConstantInt>(inst->getOperand(2))->getZExtValue(),
                        FREED
                    );
                    if(args.isInList(getLoadeeValue(inst->getPointerOperand()))) {
                        identifier.setFuncArgStat(
                            Funcs,
                            args.getOperandNum(getLoadeeValue(inst->getPointerOperand())),
                            FREED
                        );
                    }
                    generateWarning(val, "Struct element free");
                } else if (isStructFree(val)) {
                    Value * loaded_value = getStructFreedValue(val);
                    if(loaded_value != NULL){
                        if(args.isInList(loaded_value)){
                            identifier.setFuncArgStat(
                                Funcs,
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

    void Analyzer::checkAndMarkAlloc(CallInst *CI){
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
                identifier.setFuncArgStat(
                    Funcs,
                    args.getOperandNum(getLoadeeValue(inst->getPointerOperand())),
                    ALLOCATED
                );
            }
            generateWarning(CI, "Struct element malloc");
        }
    }
}
