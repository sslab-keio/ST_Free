#include "include/BaseAnalyzer.hpp"

#define isEntryPoint(F, B) &(F.getEntryBlock()) == &B ? true:false
#define UpdateIfNull(tgt, cand) (tgt) = ((tgt) == NULL ? (cand):(tgt))

namespace ST_free {
    void BaseAnalyzer::analyze(Function &F){
        setFunctionInformation(identifier.getElement(&F));
        getFunctionInformation()->setLoopInfo(loopmap->get(&F));

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

    void BaseAnalyzer::analyzeInstructions(BasicBlock &B) {
        for (Instruction &I: B){
            if(this->isReturnFunc(&I))
                getFunctionInformation()->addEndPoint(&B);

            if(AllocaInst *AI = dyn_cast<AllocaInst>(&I))
                this->analyzeAllocaInst(AI, B);
            else if (CallInst *CI = dyn_cast<CallInst>(&I))
                this->analyzeCallInst(CI, B);
            else if (StoreInst *SI = dyn_cast<StoreInst>(&I))
                this->analyzeStoreInst(SI, B);
            else if (BranchInst *BI = dyn_cast<BranchInst>(&I))
                this->analyzeBranchInst(BI, B);
        }
    }

    void BaseAnalyzer::analyzeAllocaInst(AllocaInst * AI, BasicBlock &B) {
        // this->addLocalVariable(
        //         &B,
        //         ainst->getAllocatedType(),
        //         ainst,
        //         cast<Instruction>(getFirstUser(&I)),
        //         ParentList()
        //     );
    }

    void BaseAnalyzer::analyzeStoreInst(StoreInst * SI, BasicBlock &B) {
        // if(this->isStoreToStruct(SI)){
        // }
        if(this->isStoreToStructMember(SI)) {
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

            if(isa<AllocaInst>(SI->getValueOperand())) {
                getFunctionInformation()->setAliasInBasicBlock(&B, GEle, SI->getValueOperand());
            }
        }

        if(this->isStoreFromStructMember(SI)) {
            generateWarning(SI, "is Store from struct");
            GetElementPtrInst * GEle = getStoredStructEle(SI);
            if(isa<AllocaInst>(SI->getPointerOperand())) {
                getFunctionInformation()->setAliasInBasicBlock(&B, GEle, SI->getPointerOperand());
            }
        }
    }
     
    void BaseAnalyzer::analyzeCallInst(CallInst *CI, BasicBlock &B) {
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

    void BaseAnalyzer::analyzeBranchInst(BranchInst * BI, BasicBlock &B){
        if(this->isCorrectlyBranched(BI)) {
            generateWarning(BI, "Correctly Branched");
            getFunctionInformation()->setCorrectlyBranched(&B);
        }
    }
    
    void BaseAnalyzer::checkAvailability() {
        FreedStructList fsl = getFunctionInformation()->getFreedStruct();

        // for(FreedStruct * localVar: getFunctionInformation()->getLocalVar()) {
        //     if(!getFunctionInformation()->isArgValue(localVar->getValue())){
        //         UniqueKey uk(localVar->getValue(), localVar->getType(), -1);
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
                if(vinfo != NULL) {
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
                            // getFunctionInformation()->setStructMemberArgFreed(vinfo->getValue(), vinfo->getMemberNum());
                            getFunctionInformation()->setStructMemberArgFreed(vinfo->getValue(), ParentList());
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

    bool BaseAnalyzer::isCorrectlyBranched(BranchInst * BI){
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

    void BaseAnalyzer::addLocalVariable(BasicBlock *B, Type * T, Value * V, Instruction * I, ParentList P){
        ValueInformation *vinfo = getFunctionInformation()->addVariable(V);
        getFunctionInformation()->addBasicBlockLiveVariable(B, V);
        if (StructType * strTy = dyn_cast<StructType>(T)) {
            getFunctionInformation()->addLocalVar(B, strTy, V, I, P, vinfo);

            // P.push_back(T);
            for (Type * ele: strTy->elements()) {
                if(ele->isStructTy())
                    this->addLocalVariable(B, ele, V, I, P);
            }
		}
        return;
    }

    void BaseAnalyzer::addPointerLocalVariable(BasicBlock *B, Type * T, Value * V, Instruction * I, ParentList P){
        ValueInformation *vinfo = getFunctionInformation()->addVariable(V);
        if (StructType * strTy = dyn_cast<StructType>(get_type(T))) {
            getFunctionInformation()->addLocalVar(B, strTy, V, I, P, vinfo);

            // P.push_back(T);
            for (Type * ele: strTy->elements()) {
                if(ele->isStructTy())
                    this->addLocalVariable(B, ele, V, I, P);
            }
		}
        return;
    }

    void BaseAnalyzer::analyzeDifferentFunc(Function &F) {
        /*** Push current FunctionInformation ***/
        functionStack.push(&getFunctionInformation()->getFunction());

        /*** Analyze new Function ***/
        this->analyze(F);
        // BaseAnalyzer called_function(&F, stManage, loopmap);
        // called_function.analyze();

        /*** Recover FunctionInformation ***/
        Function* tempFunc = functionStack.top();
        setFunctionInformation(identifier.getElement(tempFunc));
        functionStack.pop();
        return;
    }

    void BaseAnalyzer::addFree(Value * V, CallInst *CI, BasicBlock *B, bool isAlias, ParentList additionalParents) {
        bool isStructRelated = false;
        long index = -1;
        Value* freeValue = NULL;
        Type* memType = NULL;
        StructType* parentType = NULL;
        ParentList indexes;

        if (Instruction * val = dyn_cast<Instruction>(V)) {
            if(isStructEleFree(val) || additionalParents.size() > 0) {
                GetElementPtrInst * GEle = getFreeStructEleInfo(val);
                if (GEle != NULL) {
                    this->getStructParents(GEle, indexes);
                    if (isa<GetElementPtrInst>(GEle->getPointerOperand()))
                        GEle = getRootGEle(GEle);
                    UpdateIfNull(freeValue, getLoadeeValue(GEle->getPointerOperand()));
                }

                for(auto addParent : additionalParents) {
                    indexes.push_back(addParent);
                }

                index = indexes.back().second;
                UpdateIfNull(memType, indexes.back().first);

                if (get_type(indexes.front().first)->isStructTy())
                    UpdateIfNull(parentType, cast<StructType>(get_type(indexes.front().first)));
                isStructRelated = true;
                generateWarning(val, "Struct element free");
            }

            if (isStructFree(val)) {
                Value * loaded_value = getStructFreedValue(val);
                if (loaded_value) {
                    UpdateIfNull(freeValue, loaded_value);
                    UpdateIfNull(memType, getStructType(val));
                    if (!isAlias && !getFunctionInformation()->aliasExists(B, freeValue) && get_type(memType)->isStructTy()) {
                        getFunctionInformation()->addFreedStruct(B, get_type(memType), freeValue, CI, parentType, index != ROOT_INDEX);
                    }
                }
                isStructRelated = true;
                generateWarning(val, "Struct Free");
            } else if (isOptimizedStructFree(val)) {
                generateWarning(val, "Optimized Struct Free?");
                UpdateIfNull(freeValue, val);
                UpdateIfNull(memType, getOptimizedStructFree(val));
                if (get_type(memType)->isStructTy()) {
                    getFunctionInformation()->addFreedStruct(B, get_type(memType), freeValue, CI, parentType, index != ROOT_INDEX);
                }
                isStructRelated = true;
            }

            if (!isStructRelated) {
                UpdateIfNull(freeValue, getFreedValue(val));
                if (freeValue != NULL)
                    UpdateIfNull(memType, freeValue->getType());
                generateWarning(val, "Value Free");
            }

            if (freeValue) {
                if (getFunctionInformation()->aliasExists(B, freeValue)) {
                    Value * aliasVal = getFunctionInformation()->getAlias(B, freeValue);

                    if (GetElementPtrInst *GEle = dyn_cast<GetElementPtrInst>(aliasVal)){
                        generateWarning(CI, "Alias Free found");
                        if (V != aliasVal)
                            this->addFree(GEle, CI, B, true);
                    }
                }

                if (getFunctionInformation()->isArgValue(freeValue)) {
                    if (!parentType)
                        getFunctionInformation()->setArgFree(freeValue);
                    else if (parentType && index >= 0){
                        getFunctionInformation()->setStructMemberArgFreed(freeValue, indexes);
                    }
                }
                getFunctionInformation()->addFreeValue(B, freeValue, memType, parentType, index, indexes);
            }
        }
    }

    void BaseAnalyzer::addAlloc(CallInst *CI, BasicBlock *B) {
        Type *Ty = CI->getType();
        for (User *usr:CI->users()) {
            if (auto BitCast = dyn_cast<BitCastInst>(usr)) {
                Ty = BitCast->getDestTy();
            }
        }
        if(get_type(Ty)->isStructTy()) {
            // Value, Type
            // alias CI type to got Struct Type
        }
        // if (isStructEleAlloc(CI)) {
        //     GetElementPtrInst *inst = getAllocStructEleInfo(CI);

        //     if (inst != NULL) {
        //         if (getFunctionInformation()->isArgValue(getLoadeeValue(inst->getPointerOperand())))
        //             getFunctionInformation()->setArgAlloc(getLoadeeValue(inst->getPointerOperand()));

        //         // getFunctionInformation()->addAllocValue(
        //         //         B,
        //         //         getLoadeeValue(inst->getPointerOperand()),
        //         //         inst->getResultElementType()
        //         //         );
        //         generateWarning(CI, "Struct element malloc");
        //     }
        // } else {
        //     Value * val = getAllocatedValue(CI);
        //     if (getFunctionInformation()->isArgValue(val))
        //         getFunctionInformation()->setArgAlloc(val);
        //     this->addPointerLocalVariable(B, val->getType(), val, CI, ParentList());
        //     // getFunctionInformation()->addAllocValue(B, val, val->getType());
        //     generateWarning(CI, "Value malloc");
        // }
        return;
    }

    bool BaseAnalyzer::isReturnFunc(Instruction *I) {
        //TODO: add terminating funcs
        if(isa<ReturnInst>(I))
            return true;
        return false;
    }

    void BaseAnalyzer::copyArgStatus(Function &Func, CallInst *CI, BasicBlock &B) {
        FunctionInformation * DF = identifier.getElement(&Func);
        int ind = 0;

        for (auto arguments = CI->arg_begin(); arguments != CI->arg_end();arguments++, ind++) {
            ArgStatus *args = DF->getArgList()->getArgStatus(ind);
            this->copyArgStatusRecursively(Func, CI, B, cast<Value>(arguments), args, ind, ParentList(), true);
        }
        return;
    }

    void BaseAnalyzer::copyArgStatusRecursively(Function &Func, CallInst *CI, BasicBlock &B, Value *arg, ArgStatus *ArgStat, int ind, ParentList plist, bool isFirst) {
        if (ArgStat && ArgStat->isStruct()) {
            if (!isFirst)
                plist.push_back(pair<Type *, int>(ArgStat->getType(), ind));

            if (ArgStat->isFreed()) {
                this->addFree(arg, CI, &B, false, plist);
                Type *T = get_type(ArgStat->getType());
                if (isa<StructType>(T))
                    getFunctionInformation()->copyStructMemberFreed(T, ArgStat->getFreedList());
            }

            for (int index = 0; index < ArgStat->size(); index++) {
                this->copyArgStatusRecursively(Func, CI, B, arg, ArgStat->getStatus(index), index, plist);
            }
        }
    }

    bool BaseAnalyzer::isStoreToStructMember(StoreInst * SI) {
        if(GetElementPtrInst * gEle = dyn_cast<GetElementPtrInst>(SI->getPointerOperand())) {
            if(isa<StructType>(gEle->getSourceElementType())) {
                return true;
            }
        }
        return false;
    }

    bool BaseAnalyzer::isStoreFromStructMember(StoreInst * SI){
        if(getStoredStructEle(SI))
            return true;
        return false;
    }

    bool BaseAnalyzer::isStoreToStruct(StoreInst *SI){
        if(SI->getPointerOperandType()->isPointerTy())
            if(get_type(SI->getPointerOperandType())->isStructTy())
                return true;
            return false;
    }
    
    bool BaseAnalyzer::isStoreFromStruct(StoreInst *SI){
        if(get_type(SI->getValueOperand()->getType())->isStructTy())
            return true;
        return false;
    }

    // UniqueKey BaseAnalyzer::decodeGEPInst(GetElementPtrInst *GEle){
    //     return UniqueKey(getLoadeeValue(GEle->getPointerOperand()), GEle->getResultElementType(), getValueIndices(GEle));
    // }

    vector<string> BaseAnalyzer::decodeDirectoryName(string fname){
        vector<string> dirs;
        size_t pos;

        while ((pos = fname.find("/")) != string::npos) {
            string token = fname.substr(0, pos);
            dirs.push_back(token);
            fname.erase(0, pos + 1);
        }
        dirs.push_back(fname);

        return dirs;
    }

    void BaseAnalyzer::getStructParents(Instruction *I, vector<pair<Type *, int>> &typeList) {
        if(LoadInst *LI = dyn_cast<LoadInst>(I)) {
            if(Instruction *Inst = dyn_cast<Instruction>(LI->getPointerOperand()))
                this->getStructParents(Inst, typeList);
        } else if(GetElementPtrInst * GI = dyn_cast<GetElementPtrInst>(I)) {
            if(Instruction *Inst = dyn_cast<Instruction>(GI->getPointerOperand()))
                this->getStructParents(Inst, typeList);
            typeList.push_back(pair<Type *, int>(GI->getResultElementType(), getValueIndices(GI)));
        } else if(AllocaInst *AI = dyn_cast<AllocaInst>(I)) {
            typeList.push_back(pair<Type *, int>(AI->getAllocatedType(), ROOT_INDEX));
        }
        return;
    }
}
