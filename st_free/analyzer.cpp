#include "analyzer.hpp"
#include "determinator.hpp"
#include "support_funcs.hpp"

#define isEntryPoint(F, B) &(F.getEntryBlock()) == &B ? true:false

namespace ST_free{
    FunctionManager Analyzer::identifier;

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

    void Analyzer::analyzeInstructions(BasicBlock &B) {
        for (Instruction &I: B){
            if(this->isReturnFunc(&I))
                FEle->addEndPoint(&B);

            if(AllocaInst * ainst = dyn_cast<AllocaInst>(&I))
                this->addLocalStruct(
                        ainst->getType(),
                        ainst,
                        cast<Instruction>(getFirstUser(&I)),
                        ParentList()
                    );

            if (auto* CI = dyn_cast<CallInst>(&I)) {
                /*** get Called Function ***/
                if (Function* called_function = CI->getCalledFunction()) {
                    if (isAllocFunction(called_function)) {
                        this->addAlloc(CI, &B);
                    } else if (isFreeFunction(called_function)) {
                        for (auto arguments = CI->arg_begin(); arguments != CI->arg_end(); arguments++) {
                            this->addFree(cast<Value>(arguments), CI, &B);
                        }
                    } else {
                        this->analyzeDifferentFunc((Function &)(*called_function));
                        this->copyArgStatus((Function &)(*called_function), CI, B);
                    }
                }
            }

            if (auto *SI = dyn_cast<StoreInst>(&I)) {
                // outs() << "ValueOperand: " << *SI->getValueOperand() << "\n";
                // outs() << "PointerOperand: " << *SI->getPointerOperand() << "\n";
                // if(auto *LI = dyn_cast<LoadInst>(SI->getValueOperand())){
                //     outs() << "Load Inst: " << *LI->getPointerOperand() << "\n";
                // }
                // BBManage.incrementRefCount(*SI->getValueOperand(), *SI->getPointerOperand());
                GetElementPtrInst * GEle = getStoredStructEle(SI);
                if(GEle != NULL){
                FEle->incrementAllocatedRefCount(&B, SI->getValueOperand(), SI->getPointerOperand());
                    // outs() << "Loadee Value : " << *getLoadeeValue(GEle->getPointerOperand()) <<"\n";
                    // outs() << "Source : " << *GEle->getSourceElementType() << " " << getValueIndices(GEle) << "\n";
                }
            }
        }
    }
    
    void Analyzer::checkAvailability() {
        FreedStructList fsl = FEle->getFreedStruct();

        for(FreedStruct localVar: FEle->getLocalVar()) {
            // outs() << "Local Variable: " << * localVar.getType() << "\n";
            if(!FEle->isArgValue(localVar.getValue()))
                if(find(fsl.begin(), fsl.end(), pair<Type *, Value *>(localVar.getType(), localVar.getValue())) == fsl.end())
                    fsl.push_back(localVar);
        }

        for(FreedStruct freedStruct: fsl) {
            StructType * strTy = cast<StructType>(freedStruct.getType());
            int cPointers = strTy->getNumElements();
            vector<bool> alreadyFreed = freedStruct.getFreedMember();
            int ind = 0;
            for (Type * t: strTy->elements()) {
                if (!t->isPointerTy() 
                        || isFuncPointer(t)
                        || alreadyFreed[ind])
                    cPointers--;
                ind++;
            }

            for (BasicBlock *B: FEle->getEndPoint()) {
                for(ValueInformation vinfo: FEle->getFreeList(B)) {
                    if (vinfo.getStructType() == strTy){
                        cPointers--;
                        FEle->setStructMemberFreed(&freedStruct, vinfo.getMemberNum());
                        // if(FEle->isArgValue(vinfo.getValue()))
                        //     FEle->setStructMemberArgFreed(vinfo.getValue(), vinfo.getMemberNum());
                    }
                }
            }
            if (cPointers > 0) {
                generateError(freedStruct.getInst(), "Struct element is NOT Freed");
                // outs() << "After: " << cPointers << "\n";
            }
        }
        return;
    }

    void Analyzer::addLocalStruct(Type * T, Value * V, Instruction * I, ParentList P){
        if (StructType * strTy = dyn_cast<StructType>(get_type(T))) {
            // outs() << "Struct Type Member: " << *strTy << "\n";
            FEle->addLocalVar(strTy, V, I);

            P.push_back(T);
            for (Type * ele: strTy->elements()) {
                if(ele->isStructTy())
                    this->addLocalStruct(ele, V, I, P);
                // else if(ele->isPointerTy()){
                //     Type * extractEle = get_type(ele);
                //     if(extractEle != NULL && !FEle->localVarExists(extractEle)){
                //         this->addLocalStruct(extractEle, V, I);
                //     }
            }
        }
        return;
    }

    void Analyzer::analyzeDifferentFunc(Function &F) {
        Analyzer called_function(&F);
        called_function.analyze();
        return;
    }

    void Analyzer::addFree(Value * V, CallInst *CI, BasicBlock *B) {
        bool isStructRelated = false;
        if (Instruction * val = dyn_cast<Instruction>(V)) {
            // if (PointerType * ptr_ty = dyn_cast<PointerType>(val->getType())) {
                if(isStructEleFree(val)) {
                    GetElementPtrInst * inst = getFreeStructEleInfo(val);
                    if (inst != NULL) {
                        if (FEle->isArgValue(getLoadeeValue(inst->getPointerOperand())))
                            FEle->setArgFree(getLoadeeValue(inst->getPointerOperand()));
                        FEle->addFreeValue(
                                B,
                                getLoadeeValue(inst->getPointerOperand()),
                                inst->getResultElementType(),
                                inst->getSourceElementType(),
                                getValueIndices(inst));
                        generateWarning(val, "Struct element free");
                    }
                    isStructRelated = true;
                }
                if (isStructFree(val)) {
                    Value * loaded_value = getStructFreedValue(val);
                    if(loaded_value != NULL) {
                        if (FEle->isArgValue(loaded_value)) {
                            FEle->setArgFree(loaded_value);
                            FEle->setStructArgFree(loaded_value, get_type(loaded_value)->getStructNumElements());
                        }

                        FEle->addFreedStruct(getStructType(val), loaded_value, val);
                        FEle->addFreeValue(
                                B,
                                loaded_value,
                                getStructType(val),
                                NULL,
                                -1);
                        generateWarning(val, "Struct Free");
                    }
                    isStructRelated = true;
                }
                if(!isStructRelated) {
                    Value * loaded_value = getFreedValue(val);
                    if(loaded_value != NULL) {
                        if (FEle->isArgValue(loaded_value))
                            FEle->setArgFree(loaded_value);

                        FEle->addFreeValue(B, loaded_value);
                        generateWarning(val, "Value Free");
                    }
                }
            // }
        }
    }

    void Analyzer::addAlloc(CallInst *CI, BasicBlock *B) {
        if(isStructEleAlloc(CI)){
            GetElementPtrInst *inst = getAllocStructEleInfo(CI);

            if(inst != NULL) {
                if (FEle->isArgValue(getLoadeeValue(inst->getPointerOperand())))
                    FEle->setArgAlloc(getLoadeeValue(inst->getPointerOperand()));

                FEle->addAllocValue(
                        B,
                        getLoadeeValue(inst->getPointerOperand()),
                        inst->getResultElementType(),
                        inst->getSourceElementType(),
                        getValueIndices(inst));
                generateWarning(CI, "Struct element malloc");
            }
        } else {
            Value * val = getAllocatedValue(CI);
            if (FEle->isArgValue(val))
                FEle->setArgAlloc(val);

            FEle->addAllocValue(B, val);
            generateWarning(CI, "Value malloc");
        }
        return;
    }

    bool Analyzer::isReturnFunc(Instruction *I) {
        //TODO: add terminating funcs
        if(isa<ReturnInst>(I))
            return true;
        return false;
    }

    void Analyzer::copyArgStatus(Function &Func, CallInst *CI, BasicBlock &B) {
        int ind = 0;
        FunctionInformation * DF = identifier.getElement(&Func);

        for (auto arguments = CI->arg_begin(); arguments != CI->arg_end();arguments++, ind++) {
            if(DF->isArgFreed(ind)) {
                Type * T = get_type(cast<Value>(arguments));
                this->addFree(cast<Value>(arguments), CI, &B);
                if (isa<StructType>(T)) {
                    FEle->copyStructMemberFreed(T, DF->getStructMemberFreed(T));
                }
            }

            if(DF->isArgAllocated(ind))
                this->addAlloc(CI, &B);
        }
        return;
    }
}
