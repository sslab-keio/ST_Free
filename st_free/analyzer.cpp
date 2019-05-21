#include "analyzer.hpp"
#include "determinator.hpp"
#include "support_funcs.hpp"

#define isEntryPoint(F, B) &(F.getEntryBlock()) == &B ? true:false
// #define NOT_POINTER 1
// #define POINTER 2
// #define FREED 3


namespace ST_free{
    FunctionManager Analyzer::identifier;
    StatusList Analyzer::stat;

    void Analyzer::analyze(){
        Function & F = FEle->getFunction();

        if(!FEle->isUnanalyzed())
            return;

        FEle->setInProgress();
        
        for (BasicBlock &B: F){
            FEle->BBCollectInfo(B, isEntryPoint(F, B));
            this->analyzeInstructions(B);
        }

        this->checkAvailability();
        FEle->setAnalyzed();
        return;
    }

    void Analyzer::analyzeInstructions(BasicBlock &B){
        for (Instruction &I: B){
            if(this->isReturnFunc(&I))
                FEle->addEndPoint(&B);

            if (auto* CI = dyn_cast<CallInst>(&I)) {
                /*** get Called Function ***/
                if (Function* called_function = CI->getCalledFunction()) {
                    if (isAllocFunction(called_function)) {
                        this->addAlloc(CI, &B);
                    } else if (isFreeFunction(called_function)) {
                        for (auto arguments = CI->arg_begin(); arguments != CI->arg_end();arguments++) {
                            this->addFree(cast<Value>(arguments), CI, &B);
                        }
                    } else {
                        // generateWarning(CI, "Analyzing Diffent Function");
                        this->analyzeDifferentFunc((Function &)(*called_function));
                    }
                }
            }
        }
    }

    void Analyzer::checkAvailability() {
        for(FreedStruct freedStruct: FEle->getFreedStruct()) {
            StructType * strTy = cast<StructType>(freedStruct.first);
            unsigned cPointers = strTy->getNumElements();
            // vector<int> strList(strTy->getNumElements());
            // outs() << "Type: " <<  *strTy << "\n";

            for (Type * t: strTy->elements()) {
                if(!t->isPointerTy())
                    cPointers--;
            }

            for (BasicBlock *B: FEle->getEndPoint()) {
                for(ValueInformation vinfo: FEle->getFreeList(B)) {
                    if (vinfo.getStructType() == strTy)
                        cPointers--;
                }
            }
            if (cPointers > 0)
                outs() << "LOOKS LIKE Struct element is NOT Freed\n";
        }
        return;
    }

    void Analyzer::analyzeDifferentFunc(Function &F) {
        Analyzer called_function(&F);
        called_function.analyze();
        return;
    }

    void Analyzer::checkStructElements(Instruction * val) {
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

    void Analyzer::addFree(Value * V, CallInst *CI, BasicBlock *B) {
        if (Instruction * val = dyn_cast<Instruction>(V)) {
            if (PointerType * ptr_ty = dyn_cast<PointerType>(val->getType())) {
                if(isStructEleFree(val)) {
                    GetElementPtrInst * inst = getFreeStructEleInfo(CI);
                    if (inst != NULL) {
                        FEle->addFreeValue(
                                B, 
                                getLoadeeValue(inst->getPointerOperand()), 
                                inst->getResultElementType(), 
                                inst->getSourceElementType(),
                                getValueIndices(inst));
                        generateWarning(val, "Struct element free");
                    }
                } else if (isStructFree(val)) {
                    Value * loaded_value = getStructFreedValue(val);
                    if(loaded_value != NULL) {
                        FEle->addFreedStruct(getStructType(val), loaded_value);
                        FEle->addFreeValue(
                                B, 
                                loaded_value,
                                getStructType(val),
                                NULL,
                                -1);
                        generateWarning(val, "Struct Free");
                    }
                //     this->checkStructElements(val);
                }
            }
        }
    }

    void Analyzer::addAlloc(CallInst *CI, BasicBlock *B) {
        if(isStructEleAlloc(CI)){
            GetElementPtrInst *inst = getAllocStructEleInfo(CI);

            if(inst != NULL) {
                FEle->addAllocValue(
                        B,
                        getLoadeeValue(inst->getPointerOperand()),
                        inst->getResultElementType(),
                        inst->getSourceElementType(),
                        getValueIndices(inst));
                generateWarning(CI, "Struct element malloc");
            }
        }
        return;
    }

    bool Analyzer::isReturnFunc(Instruction *I){
        //TODO: add terminating funcs 
        if(isa<ReturnInst>(I))
            return true;
        return false;
    }
}
