#include "include/analyzer.hpp"

#define isEntryPoint(F, B) &(F.getEntryBlock()) == &B ? true:false
#define UpdateIfNull(tgt, cand) (tgt) = ((tgt) == NULL ? (cand):(tgt))

namespace ST_free {
    FunctionManager Analyzer::identifier;

    void Analyzer::analyze(){
        Function & F = getFunctionInformation()->getFunction();

        if(!getFunctionInformation()->isUnanalyzed())
            return;
        getFunctionInformation()->setInProgress();

        for (BasicBlock &B: F){
            getFunctionInformation()->BBCollectInfo(B, isEntryPoint(F, B));
            getFunctionInformation()->setLoopBlock(B);
            this->analyzeInstructions(B);
            getFunctionInformation()->updateSuccessorBlock(B);
        }

        this->checkAvailability();
        getFunctionInformation()->setAnalyzed();

        return;
    }

    void Analyzer::analyzeInstructions(BasicBlock &B) {
        for (Instruction &I: B){
            if(this->isReturnFunc(&I))
                getFunctionInformation()->addEndPoint(&B);

            if(AllocaInst *AI = dyn_cast<AllocaInst>(&I)) {
                this->analyzeAllocaInst(AI, B);
            }
            else if (CallInst *CI = dyn_cast<CallInst>(&I)) {
                this->analyzeCallInst(CI, B);
            }
            else if (StoreInst *SI = dyn_cast<StoreInst>(&I)) {
                this->analyzeStoreInst(SI, B);
            }
            else if (BranchInst *BI = dyn_cast<BranchInst>(&I)) {
                this->analyzeBranchInst(BI, B);
            }
        }
    }

    void Analyzer::analyzeAllocaInst(AllocaInst * AI, BasicBlock &B){
        // this->addLocalVariable(
        //         &B,
        //         ainst->getAllocatedType(),
        //         ainst,
        //         cast<Instruction>(getFirstUser(&I)),
        //         ParentList()
        //     );
    }

    void Analyzer::analyzeStoreInst(StoreInst * SI, BasicBlock &B){
        // if(this->isStoreToStruct(SI)){
        // }
        if(this->isStoreToStructMember(SI)){
            generateWarning(SI, "is Store to struct");
            GetElementPtrInst * GEle = getStoredStruct(SI);
            stManage->addStore(cast<StructType>(GEle->getSourceElementType()), getValueIndices(GEle));

            if(isa<GlobalValue>(SI->getValueOperand())) {
                generateWarning(SI, "GolbalVariable Store");
                stManage->addGlobalVarStore(
                        cast<StructType>(GEle->getSourceElementType()), 
                        getValueIndices(GEle)
                    );
            }
            // if(isa<AllocaInst>(SI->getValueOperand())){
            //     getFunctionInformation()->setAliasInBasicBlock(&B, GEle, SI->getValueOperand());
            // }
        }

        if(this->isStoreFromStructMember(SI)) {
            generateWarning(SI, "is Store from struct");
            GetElementPtrInst * GEle = getStoredStructEle(SI);
            if(isa<AllocaInst>(SI->getPointerOperand())) {
                getFunctionInformation()->setAliasInBasicBlock(&B, GEle, SI->getPointerOperand());
            }
        }
    }
     
    void Analyzer::analyzeCallInst(CallInst *CI, BasicBlock &B) {
        if (Function* called_function = CI->getCalledFunction()) {
            if (isAllocFunction(called_function)) {
                Value * val = getAllocatedValue(CI);
                if(val != NULL) 
                    if(StructType * strTy = dyn_cast<StructType>(get_type(val->getType()))) {
                        stManage->addAlloc(strTy);
                    }
                // this->addAlloc(CI, &B);
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

    void Analyzer::analyzeBranchInst(BranchInst * BI, BasicBlock &B){
        if(this->isCorrectlyBranched(BI)) {
            generateWarning(BI, "Correctly Branched");
            getFunctionInformation()->setCorrectlyBranched(&B);
        }
    }
    
    void Analyzer::checkAvailability() {
        FreedStructList fsl = getFunctionInformation()->getFreedStruct();

        // for(FreedStruct * localVar: getFunctionInformation()->getLocalVar()) {
        //     if(!getFunctionInformation()->isArgValue(localVar->getValue())){
        //         uniqueKey uk(localVar->getValue(), localVar->getType(), -1);
        //         if(find_if(fsl.begin(), fsl.end(), 
        //                     [uk](FreedStruct *f){return *f == uk;}) == fsl.end())
        //             fsl.push_back(localVar);
        //     }
        // }

        for(FreedStruct * freedStruct: fsl) {
            StructType * strTy = cast<StructType>(freedStruct->getType());
            int cPointers = strTy->getNumElements();
            vector<bool> alreadyFreed = freedStruct->getFreedMember();
            for (int ind = 0; ind < strTy->getNumElements(); ind++) {
                Type *t = strTy->getElementType(ind);
                if (!t->isPointerTy() 
                        || isFuncPointer(t)
                        || alreadyFreed[ind])
                    continue;

                ValueInformation *vinfo = getFunctionInformation()->getValueInfo(freedStruct->getValue(), t, ind);
                if(vinfo != NULL){
                    bool isFreed = false;
                    if(getFunctionInformation()->isFreedInBasicBlock(freedStruct->getFreedBlock(), vinfo->getValue(), t, ind)
                            || getFunctionInformation()->isCorrectlyBranchedFreeValue(freedStruct->getFreedBlock(), vinfo->getValue(), t, ind)) {
                        isFreed = true;
                    // }
                    // else if (!vinfo->noRefCount()) {
                        // bool storedValueFreed = false;
                        // for(Value * val : vinfo->getAliasList()){
                        //     if(getFunctionInformation()->isFreedInBasicBlock(freedStruct->getFreedBlock(), val, val->getType(), -1)
                        //         || getFunctionInformation()->isCorrectlyBranchedFreeValue(freedStruct->getFreedBlock(), val, val->getType(), -1)){
                        //         storedValueFreed = true;
                        //         break;
                        //     }
                        // }
                        // if(storedValueFreed){
                        //     generateWarning("Found Store In Loop\n");
                        //     freedStruct->setStoredInLoop(ind);
                        //     cPointers--;
                        // }
                    }

                    if(isFreed) {
                        getFunctionInformation()->setStructMemberFreed(freedStruct, vinfo->getMemberNum());
                        if(getFunctionInformation()->isArgValue(vinfo->getValue())) {
                            getFunctionInformation()->setStructMemberArgFreed(vinfo->getValue(), vinfo->getMemberNum());
                        }
                    }
                }
            }

            if (!getFunctionInformation()->isArgValue(freedStruct->getValue())){
                stManage->addCandidateValue(&(getFunctionInformation()->getFunction()), strTy, freedStruct);
            }
        }
        return;
    }

    bool Analyzer::isCorrectlyBranched(BranchInst * BI){
        if(BI->isConditional() && BI->getCondition() != NULL){
            if(auto * CmpI = dyn_cast<CmpInst>(BI->getCondition())){
                if(auto *LI = dyn_cast<LoadInst>(CmpI->getOperand(0)))
                    if(string(LI->getPointerOperand()->getName()).find("ref") != string::npos)
                        return true;
                if(isa<ConstantPointerNull>(CmpI->getOperand(1))){
                    return true;
                }
            }
        }
        return false;
    }

    void Analyzer::addLocalVariable(BasicBlock *B, Type * T, Value * V, Instruction * I, ParentList P){
        ValueInformation *vinfo = getFunctionInformation()->addVariable(V);
        getFunctionInformation()->addBasicBlockLiveVariable(B, V);
        if (StructType * strTy = dyn_cast<StructType>(T)) {
            getFunctionInformation()->addLocalVar(B, strTy, V, I, P, vinfo);

            P.push_back(T);
            for (Type * ele: strTy->elements()) {
                if(ele->isStructTy())
                    this->addLocalVariable(B, ele, V, I, P);
            }
		}
        return;
    }

    void Analyzer::addPointerLocalVariable(BasicBlock *B, Type * T, Value * V, Instruction * I, ParentList P){
        ValueInformation *vinfo = getFunctionInformation()->addVariable(V);
        if (StructType * strTy = dyn_cast<StructType>(get_type(T))) {
            getFunctionInformation()->addLocalVar(B, strTy, V, I, P, vinfo);

            P.push_back(T);
            for (Type * ele: strTy->elements()) {
                if(ele->isStructTy())
                    this->addLocalVariable(B, ele, V, I, P);
            }
		}
        return;
    }

    void Analyzer::analyzeDifferentFunc(Function &F) {
        Analyzer called_function(&F, stManage, loopmap);
        called_function.analyze();
        return;
    }

    void Analyzer::addFree(Value * V, CallInst *CI, BasicBlock *B, bool isAlias) {
        bool isStructRelated = false;
        long index = -1;
        Value* freeValue = NULL;
        Type* memType = NULL;
        StructType* parentType = NULL;

        if (Instruction * val = dyn_cast<Instruction>(V)) {
            if(isStructEleFree(val)) {
                GetElementPtrInst * GEle = getFreeStructEleInfo(val);
                if (GEle != NULL) {
                    UpdateIfNull(freeValue, getLoadeeValue(GEle->getPointerOperand()));
                    UpdateIfNull(memType, GEle->getResultElementType());
                    if(GEle->getSourceElementType()->isStructTy())
                        UpdateIfNull(parentType, cast<StructType>(GEle->getSourceElementType()));
                    index = getValueIndices(GEle);
                }
                isStructRelated = true;
                generateWarning(val, "Struct element free");
            }

            if(isStructFree(val)) {
                Value * loaded_value = getStructFreedValue(val);
                if(loaded_value) {
                    UpdateIfNull(freeValue, loaded_value);
                    UpdateIfNull(memType, getStructType(val));
                    if(!isAlias && !getFunctionInformation()->aliasExists(B, freeValue))
                        getFunctionInformation()->addFreedStruct(B, getStructType(val), freeValue, CI, parentType);
                }
                isStructRelated = true;
                generateWarning(val, "Struct Free");
            }

            if(!isStructRelated) {
                UpdateIfNull(freeValue, getFreedValue(val));
                if(freeValue != NULL)
                    UpdateIfNull(memType, freeValue->getType());
                generateWarning(val, "Value Free");
            }

            if(freeValue) {
                if(getFunctionInformation()->aliasExists(B, freeValue)) {
                    Value * aliasVal = getFunctionInformation()->getAlias(B, freeValue);
                    if(GetElementPtrInst *GEle = dyn_cast<GetElementPtrInst>(aliasVal)){
                        generateWarning(CI, "Alias Free found");
                        this->addFree(GEle, CI, B, true);
                    }
                }
                if (getFunctionInformation()->isArgValue(freeValue)) {
                    getFunctionInformation()->setArgFree(freeValue);
                    if(memType->isStructTy())
                        getFunctionInformation()->setStructArgFree(freeValue, get_type(freeValue)->getStructNumElements());
                    if(parentType && index >= 0){
                        // getFunctionInformation()->setStructMemberArgFreed(freeValue, index);
                        outs() << "struct member arg freed\n";
                    }
                }
                getFunctionInformation()->addFreeValue(B, freeValue, memType, parentType, index);
            }
        }
    }

    void Analyzer::addAlloc(CallInst *CI, BasicBlock *B) {
        if(isStructEleAlloc(CI)){
            GetElementPtrInst *inst = getAllocStructEleInfo(CI);

            if(inst != NULL) {
                if (getFunctionInformation()->isArgValue(getLoadeeValue(inst->getPointerOperand())))
                    getFunctionInformation()->setArgAlloc(getLoadeeValue(inst->getPointerOperand()));

                // getFunctionInformation()->addAllocValue(
                //         B,
                //         getLoadeeValue(inst->getPointerOperand()),
                //         inst->getResultElementType()
                //         );
                generateWarning(CI, "Struct element malloc");
            }
        } else {
            Value * val = getAllocatedValue(CI);
            if (getFunctionInformation()->isArgValue(val))
                getFunctionInformation()->setArgAlloc(val);
            this->addPointerLocalVariable(B, val->getType(), val, CI, ParentList());
            // getFunctionInformation()->addAllocValue(B, val, val->getType());
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
        FunctionInformation * DF = identifier.getElement(&Func);
        int ind = 0;

        for (auto arguments = CI->arg_begin(); arguments != CI->arg_end();arguments++, ind++) {
            if(DF->isArgFreed(ind)) {
                Type * T = get_type(cast<Value>(arguments));
                this->addFree(cast<Value>(arguments), CI, &B);
                if (isa<StructType>(T)) {
                    getFunctionInformation()->copyStructMemberFreed(T, DF->getStructMemberFreed(T));
                }
            }

            if(DF->isArgAllocated(ind))
                this->addAlloc(CI, &B);
        }
        return;
    }

    bool Analyzer::isStoreToStructMember(StoreInst * SI){
        if(GetElementPtrInst * gEle = dyn_cast<GetElementPtrInst>(SI->getPointerOperand())) {
            if(isa<StructType>(gEle->getSourceElementType())) {
                return true;
            }
        }
        return false;
    }

    bool Analyzer::isStoreFromStructMember(StoreInst * SI){
        if(getStoredStructEle(SI))
            return true;
        return false;
    }

    bool Analyzer::isStoreToStruct(StoreInst *SI){
        if(SI->getPointerOperandType()->isPointerTy())
            if(get_type(SI->getPointerOperandType())->isStructTy())
                return true;
            return false;
    }

    uniqueKey Analyzer::decodeGEPInst(GetElementPtrInst *GEle){
        return uniqueKey(getLoadeeValue(GEle->getPointerOperand()), GEle->getResultElementType(), getValueIndices(GEle));
    }
}
