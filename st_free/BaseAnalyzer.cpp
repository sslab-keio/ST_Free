#include "include/BaseAnalyzer.hpp"

#define isEntryPoint(F, B) &(F.getEntryBlock()) == &B ? true:false
#define UpdateIfNull(tgt, cand) (tgt) = ((tgt) == NULL ? (cand):(tgt))

namespace ST_free {
    void BaseAnalyzer::analyzeAdditionalUnknowns(Function &F) {
        for (BasicBlock &B: F) {
            for (Instruction &I: B) {
                if (StoreInst *SI = dyn_cast<StoreInst>(&I))
                    this->checkAndChangeActualAuthority(SI);
            }
        }
        return;
    }

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
            stManage->addStore(cast<StructType>(GEle->getSourceElementType()), getValueIndices(GEle).back());

            if(isa<GlobalValue>(SI->getValueOperand())) {
                generateWarning(SI, "GolbalVariable Store");
                stManage->addGlobalVarStore(
                        cast<StructType>(GEle->getSourceElementType()), 
                        getValueIndices(GEle).back()
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
    void BaseAnalyzer::analyzeBitCastInst(BitCastInst *BCI, BasicBlock &B) {
        Type *tgtTy = get_type(BCI->getDestTy());
        if (get_type(BCI->getSrcTy())->isIntegerTy() && tgtTy->isStructTy()) {
            Value *V = BCI->getOperand(0);
            getFunctionInformation()->addAliasedType(V, tgtTy);
        }
        return;
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
        struct collectedInfo info;

        if (Instruction* val = dyn_cast<Instruction>(V)) {
            if(isStructEleFree(val) || additionalParents.size() > 0) {
                this->collectStructMemberFreeInfo(val, info, additionalParents);
                // GetElementPtrInst *GEle = getFreeStructEleInfo(val);
                // if (GEle != NULL) {
                //     this->getStructParents(GEle, info.indexes);
                //     // indexes.push_back(pair<Type*, int>(extractResultElementType(GEle), ROOT_INDEX));
                //     GetElementPtrInst *tmpGEle = GEle;
                //     if (isa<GetElementPtrInst>(GEle->getPointerOperand()))
                //         tmpGEle = getRootGEle(GEle);
                //     UpdateIfNull(info.freeValue, getLoadeeValue(tmpGEle->getPointerOperand()));
                // }

                // for(auto addParent : additionalParents) {
                //     info.indexes.push_back(addParent);
                // }

                // if (info.indexes.size() > 0) {
                //     info.index = info.indexes.back().second;
                    
                //     if (auto StTy = dyn_cast<StructType>(get_type(info.indexes.back().first))) {
                //         if(ROOT_INDEX < info.index && info.index < StTy->getNumElements())
                //             UpdateIfNull(info.memType, StTy->getElementType(info.index));
                //     }

                //     if (get_type(info.indexes.front().first)->isStructTy())
                //         UpdateIfNull(info.parentType, cast<StructType>(get_type(info.indexes.front().first)));

                //     info.isStructRelated = true;
                //     generateWarning(val, "Struct element free");
                // }
            }

            if (isStructFree(val)) {
                generateWarning(CI, "Struct Free");
                this->collectStructFreeInfo(val, info);
                // Value *loaded_value = getStructFreedValue(val);
                // if (loaded_value) {
                //     UpdateIfNull(info.freeValue, loaded_value);
                //     UpdateIfNull(info.memType, getStructType(val));
                // }
                // info.isStructRelated = true;
            } else if (isOptimizedStructFree(val)) {
                this->collectOptimizedStructFreeInfo(val, info);
                generateWarning(CI, "Optimized Struct Free");
                // UpdateIfNull(info.freeValue, val);
                // UpdateIfNull(info.memType, getOptimizedStructFree(val));
                // info.isStructRelated = true;
            }

            if (!info.isStructRelated) {
                this->collectSimpleFreeInfo(val, info);
                // UpdateIfNull(info.freeValue, getFreedValue(val));
                // if (info.freeValue != NULL)
                //     UpdateIfNull(info.memType, info.freeValue->getType());
                // generateWarning(val, "Value Free");
            }

            if (info.freeValue) {
                if (getFunctionInformation()->aliasExists(B, info.freeValue)) {
                    Value * aliasVal = getFunctionInformation()->getAlias(B, info.freeValue);

                    if (GetElementPtrInst *GEle = dyn_cast<GetElementPtrInst>(aliasVal)) {
                        generateWarning(CI, "Alias Free found");
                        if (V != aliasVal)
                            this->addFree(GEle, CI, B, true);
                    }
                }

                if (getFunctionInformation()->isArgValue(info.freeValue)) {
                    if (!info.parentType)
                        getFunctionInformation()->setArgFree(info.freeValue);
                    else if (info.parentType && info.index >= 0)
                        getFunctionInformation()->setStructMemberArgFreed(info.freeValue, info.indexes);
                }

                ValueInformation *valInfo = getFunctionInformation()->addFreeValue(B, info.freeValue, info.memType, info.parentType, info.index, info.indexes);
                if (!isAlias 
                        && !getFunctionInformation()->aliasExists(B, info.freeValue)
                        && info.memType
                        && get_type(info.memType)->isStructTy()
                        && this->isAuthorityChained(info.indexes)
                    ) {
                    getFunctionInformation()->addFreedStruct(B, get_type(info.memType), info.freeValue, CI, info.parentType, valInfo, info.index != ROOT_INDEX);

                    /*** Look for any statically allcated struct type,
                     * and add them to freed struct as well ***/
#if defined(OPTION_NESTED)
                    StructType* StTy = cast<StructType>(get_type(info.memType));
                    int memIndex = 0;
                    for (auto ele : StTy->elements()) {
                        if (ele->isStructTy()) {
                            additionalParents.push_back(pair<Type*, int>(ele, memIndex++));
                            this->addFree(V, CI, B, false, additionalParents);
                            additionalParents.pop_back();
                        }
                    }
#endif
                }
            }
        }
    }

    void BaseAnalyzer::addAlloc(CallInst *CI, BasicBlock *B) {
        Type *Ty = CI->getType();
        for (User *usr:CI->users()) {
            // if (auto BitCast = dyn_cast<BitCastInst>(usr)) {
            if (auto CastI = dyn_cast<CastInst>(usr)) {
                Ty = CastI->getDestTy();
            }
        }
        if(get_type(Ty)->isStructTy()) {
            getFunctionInformation()->addAliasedType(CI, Ty);
            getStructManager()->addAlloc(cast<StructType>(get_type(Ty)));
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

    bool BaseAnalyzer::isStoreToStruct(StoreInst *SI) {
        if(SI->getPointerOperandType()->isPointerTy())
            if(get_type(SI->getPointerOperandType())->isStructTy())
                return true;
            return false;
    }
    
    bool BaseAnalyzer::isStoreFromStruct(StoreInst *SI) {
        if(get_type(SI->getValueOperand()->getType())->isStructTy())
            return true;
        return false;
    }

    void BaseAnalyzer::checkAndChangeActualAuthority(StoreInst *SI) {
        vector<CastInst *> CastInsts;
        if(this->isStoreToStructMember(SI)) {
            GetElementPtrInst * GEle = getStoredStruct(SI);
            if(GEle != NULL && isa<StructType>(GEle->getSourceElementType())) {
                generateWarning(SI, "Found StoreInst to struct member");

                if (auto PN = dyn_cast<PHINode>(SI->getValueOperand())) {
                    generateWarning(SI, "is PhiInst");
                    for (int i = 0; i < PN->getNumIncomingValues(); i++) {
                        if (auto CI = dyn_cast<CastInst>(PN->getIncomingValue(i)))
                            CastInsts.push_back(CI);
                    }
                } else if (auto CI = dyn_cast<CastInst>(SI->getValueOperand())) {
                    CastInsts.push_back(CI);
                }
                
                for(auto CI: CastInsts)
                    this->changeAuthority(SI, CI, GEle);
            }
        }
    }

    void BaseAnalyzer::changeAuthority(StoreInst *SI, CastInst *CI, GetElementPtrInst *GEle) {
        ParentList indexes;
        generateWarning(SI, "is Casted Store");
        this->getStructParents(GEle, indexes);
        if (this->isAuthorityChained(vector<pair<Type *, int>>(indexes.end() - 1, indexes.end()))
                && !this->isAllocCast(CI)) {
            if (StructType *StTy = dyn_cast<StructType>(indexes.back().first)) {
                generateWarning(SI, "Change back to Unknown");
                getStructManager()->get(StTy)->setMemberStatUnknown(indexes.back().second);
            }
        }
        return;
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
            // typeList.push_back(pair<Type *, int>(GI->getResultElementType(), getValueIndices(GI)));
            vector<pair<Type*, long>> decoded_vals = this->decodeGEPInst(GI);
            for (auto dec : decoded_vals) {
                if (dec.first != NULL && dec.second != ROOT_INDEX)
                    typeList.push_back(pair<Type *, int>(dec.first, dec.second));
            }
        } else if(AllocaInst *AI = dyn_cast<AllocaInst>(I)) {
            // typeList.push_back(pair<Type *, int>(AI->getAllocatedType(), ROOT_INDEX));
        }
        return;
    }

    long BaseAnalyzer::getMemberIndiceFromByte(StructType * STy, uint64_t byte){
        const StructLayout* sl = this->getStructLayout(STy);
        if (sl != NULL)
            return sl->getElementContainingOffset(byte);
        return ROOT_INDEX;
    }

    vector<long> BaseAnalyzer::getValueIndices(GetElementPtrInst* inst) {
        // auto idx_itr = inst->idx_end() - 1;
        long indice = ROOT_INDEX;
        vector<long> indices;

        for(auto idx_itr = inst->idx_begin() + 1; idx_itr != inst->idx_end(); idx_itr++) {
            if(ConstantInt *cint = dyn_cast<ConstantInt>(idx_itr->get()))
                indice = cint->getSExtValue();
            else
                indice = ROOT_INDEX;
            
            indices.push_back(indice);
        }

        return indices;
    }

    GetElementPtrInst* BaseAnalyzer::getRootGEle(GetElementPtrInst *GEle) {
        GetElementPtrInst *tgt = GEle;
        while(isa<GetElementPtrInst>(tgt->getPointerOperand())){
            tgt = cast<GetElementPtrInst>(tgt->getPointerOperand());
        }
        return tgt;
    }

    bool BaseAnalyzer::isStructEleAlloc(Instruction * val){
        for (User *usr: val->users()){
            User * tmp_usr = usr;
            if(!isa<StoreInst>(usr)){
                for(User * neo_usr: usr->users()){
                    if(isa<StoreInst>(neo_usr)){
                        tmp_usr = neo_usr;
                        break;
                    }
                }
            }
            if(StoreInst * str_inst = dyn_cast<StoreInst>(tmp_usr)){
                Value * tgt_op = str_inst->getOperand(1);
                if(GetElementPtrInst * inst = dyn_cast<GetElementPtrInst>(tgt_op)){
                    return true;
                }
            }
        }
        return false;
    }
    Value* BaseAnalyzer::getAllocatedValue(Instruction * val){
        for (User *usr: val->users()){
            User * tmp_usr = usr;
            if(!isa<StoreInst>(usr)){
                for(User * neo_usr: usr->users()){
                    if(isa<StoreInst>(neo_usr)){
                        tmp_usr = neo_usr;
                        break;
                    }
                }
            }
            if(StoreInst * str_inst = dyn_cast<StoreInst>(tmp_usr)){
                Value * tgt_op = str_inst->getOperand(1);
                return tgt_op;
            }
        }
        return NULL;
    }

    GetElementPtrInst * BaseAnalyzer::getAllocStructEleInfo(Instruction * val){
        for (User *usr: val->users()){
            User * tmp_usr = usr;
            if(!isa<StoreInst>(usr)){
                for(User * neo_usr: usr->users()){
                    if(isa<StoreInst>(neo_usr)){
                        tmp_usr = neo_usr;
                        break;
                    }
                }
            }
            if(StoreInst * str_inst = dyn_cast<StoreInst>(tmp_usr)){
                Value * tgt_op = str_inst->getOperand(1);
                if(GetElementPtrInst * inst = dyn_cast<GetElementPtrInst>(tgt_op)){
                    return inst;
                }
            }
        }
        return NULL;
    }

    bool BaseAnalyzer::isStructEleFree(Instruction *val){
        if(isa<GetElementPtrInst>(val))
            return true;

        LoadInst * l_inst = find_load(val);
        if(l_inst != NULL && l_inst->getOperandList() != NULL){
            Value *V = l_inst->getPointerOperand();
            if (auto bit_cast_inst = dyn_cast<BitCastInst>(V)) {
                generateWarning(val, "found BitCast");
                V = bit_cast_inst->getOperand(0);
            }
            if (auto GEle = dyn_cast<GetElementPtrInst>(V)) {
                return true;
            }
            // generateError(val , "Found load inst operandlist");
            // for(Use &U : l_inst->operands()){
            //     if(GetElementPtrInst * inst = dyn_cast<GetElementPtrInst>(U)){
            //         return true;
            //     }
            // }
        }
        return false;
    }

    GetElementPtrInst* BaseAnalyzer::getFreeStructEleInfo(Instruction * val){
         if(auto GEle = dyn_cast<GetElementPtrInst>(val))
            return GEle;

        LoadInst * l_inst = find_load(val);
        if(l_inst != NULL && l_inst->getOperandList() != NULL){
            Value *V = l_inst->getPointerOperand();
            if (auto bit_cast_inst = dyn_cast<BitCastInst>(V)) {
                generateWarning(val, "found BitCast");
                V = bit_cast_inst->getOperand(0);
            }
            if (auto GEle = dyn_cast<GetElementPtrInst>(V)) {
                return GEle;
            }
        }
        return NULL;       
    }

    bool BaseAnalyzer::isStructFree(Instruction * val) {
        // if (auto BCI = dyn_cast<BitCastInst>(val)) {
        //     if(get_type(BCI->getSrcTy())->isStructTy())
        //         return true;
        // }
        if(getStructFreedValue(val) != NULL)
            return true;
        return false;
    }

    bool BaseAnalyzer::isOptimizedStructFree(Instruction *I) {
        return getFunctionInformation()->aliasedTypeExists(I);
    }

    Type* BaseAnalyzer::getOptimizedStructFree(Instruction *I) {
        return getFunctionInformation()->getAliasedType(I);
    }

    Type * BaseAnalyzer::getStructType(Instruction* val) {
        LoadInst *load_inst = find_load(val);
        if (load_inst && load_inst->getOperandList() != NULL) {
            Type * tgt_type = get_type(load_inst->getPointerOperandType());
            if (tgt_type && get_type(tgt_type)->isStructTy())
                return tgt_type;
        }
        return NULL;
    }

    bool BaseAnalyzer::isFuncPointer(Type * t) {
        Type * tgt = get_type(t);
        if(tgt->isFunctionTy())
            return true;
        return false;
    }

    Value* BaseAnalyzer::getStructFreedValue(Instruction* val) {
        LoadInst *load_inst = find_load(val);
        if (load_inst && load_inst->getOperandList() != NULL) {
            Type * tgt_type = get_type(load_inst->getPointerOperandType());
            if (tgt_type)
                if(isa<StructType>(get_type(tgt_type))) {
                return getLoadeeValue(load_inst);
            }
        }
        return NULL;
    }

    Value * BaseAnalyzer::getFreedValue(Instruction * val) {
        LoadInst *load_inst = find_load(val);
        if (load_inst != NULL && load_inst->getOperandList() != NULL)
            return getLoadeeValue(load_inst);
        return NULL;
    }

    GetElementPtrInst *  BaseAnalyzer::getStoredStructEle(StoreInst * SI) {
        if(auto LInst = dyn_cast<LoadInst>(SI->getValueOperand()))
            if(auto GEle = dyn_cast<GetElementPtrInst>(LInst->getPointerOperand()))
                return GEle;
        return NULL;
    }

    GetElementPtrInst * BaseAnalyzer::getStoredStruct(StoreInst *SI){
        if(GetElementPtrInst * gepi = dyn_cast<GetElementPtrInst>(SI->getPointerOperand()))
            return gepi;
        return NULL;
    }

    vector<pair<Type*, long>> BaseAnalyzer::decodeGEPInst(GetElementPtrInst *GEle) {
        Type* Ty = GEle->getSourceElementType();
        vector<long> indice = getValueIndices(GEle);
        vector<pair<Type *, long>> decoded;

        for(long ind: indice) {
            long index = ind;
            if(Ty && index != ROOT_INDEX) {
                if (Ty->isIntegerTy()) {
                    if(getFunctionInformation()->aliasedTypeExists(GEle->getPointerOperand()) && decoded.empty())
                        Ty = getFunctionInformation()->getAliasedType(GEle->getPointerOperand());
                    if (Ty && get_type(Ty)->isStructTy())
                        index = getMemberIndiceFromByte(cast<StructType>(get_type(Ty)), index);
                }
                decoded.push_back(pair<Type *, long>(Ty, index));
                if(auto StTy = dyn_cast<StructType>(Ty))
                    Ty = StTy->getElementType(index);
            }
        }

        return decoded;
    }

    Type* BaseAnalyzer::extractResultElementType(GetElementPtrInst *GEle) {
        Type *Ty = GEle->getResultElementType();

        if (get_type(Ty)->isIntegerTy()) {
            for(User* usr: GEle->users()) {
                if (auto BCI = dyn_cast<BitCastInst>(usr)) {
                    Ty = get_type(BCI->getDestTy());
                }
            }
        }
        return Ty;
    }
    
    bool BaseAnalyzer::isAuthorityChained(ParentList pt) {
        for (pair<Type *, long> ele : pt) {
            if (auto StTy = dyn_cast<StructType>(get_type(ele.first))) {
                if (!stManage->structHoldsAuthority(StTy, ele.second))
                    return false;
            }
        }
        return true;
    }

    bool BaseAnalyzer::isAllocCast(CastInst *cast) {
        if (auto CI = dyn_cast<CallInst>(cast->getOperand(0)))
            if (isAllocFunction(CI->getCalledFunction())) 
                return true;
        return false;
    }

    void BaseAnalyzer::collectStructMemberFreeInfo(Instruction *I, struct BaseAnalyzer::collectedInfo &info, ParentList &additionalParents) {
        GetElementPtrInst *GEle = getFreeStructEleInfo(I);
        if (GEle != NULL) {
            this->getStructParents(GEle, info.indexes);
            // indexes.push_back(pair<Type*, int>(extractResultElementType(GEle), ROOT_INDEX));
            GetElementPtrInst *tmpGEle = GEle;
            if (isa<GetElementPtrInst>(GEle->getPointerOperand()))
                tmpGEle = getRootGEle(GEle);
            UpdateIfNull(info.freeValue, getLoadeeValue(tmpGEle->getPointerOperand()));
        }

        for(auto addParent : additionalParents) {
            info.indexes.push_back(addParent);
        }

        if (info.indexes.size() > 0) {
            info.index = info.indexes.back().second;
            
            if (auto StTy = dyn_cast<StructType>(get_type(info.indexes.back().first))) {
                if(ROOT_INDEX < info.index && info.index < StTy->getNumElements())
                    UpdateIfNull(info.memType, StTy->getElementType(info.index));
            }

            if (get_type(info.indexes.front().first)->isStructTy())
                UpdateIfNull(info.parentType, cast<StructType>(get_type(info.indexes.front().first)));

            info.isStructRelated = true;
            generateWarning(I, "Struct element free");
        }
        return;
    }

    void BaseAnalyzer::collectSimpleFreeInfo(Instruction *I, struct BaseAnalyzer::collectedInfo &info) {
        UpdateIfNull(info.freeValue, getFreedValue(I));
        if (info.freeValue != NULL)
            UpdateIfNull(info.memType, info.freeValue->getType());
        generateWarning(I, "Value Free");
        return;
    } 

    void BaseAnalyzer::collectStructFreeInfo(Instruction *I, struct BaseAnalyzer::collectedInfo &info) {
        Value *loaded_value = getStructFreedValue(I);
        if (loaded_value) {
            UpdateIfNull(info.freeValue, loaded_value);
            UpdateIfNull(info.memType, getStructType(I));
        }
        info.isStructRelated = true;
        return;
    }

    void BaseAnalyzer::collectOptimizedStructFreeInfo(Instruction *I, struct BaseAnalyzer::collectedInfo &info){
        UpdateIfNull(info.freeValue, I);
        UpdateIfNull(info.memType, getOptimizedStructFree(I));
        info.isStructRelated = true;
        return;
    }
}
