#include "include/analyzer.hpp"

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
        FEle->setLoopBlock(B);

        for (Instruction &I: B){
            if(this->isReturnFunc(&I))
                FEle->addEndPoint(&B);

            if(AllocaInst * ainst = dyn_cast<AllocaInst>(&I)){
                this->analyzeAllocaInst(ainst, B);
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
        if(isStoreToStruct(SI)){
            generateWarning(SI, "is Store to struct");
            GetElementPtrInst * GEle = getStoredStruct(SI);
            stManage->addStore(cast<StructType>(GEle->getSourceElementType()), getValueIndices(GEle));

            if(isa<GlobalValue>(SI->getValueOperand())){
                stManage->addGlobalVarStore(
                        cast<StructType>(GEle->getSourceElementType()), 
                        getValueIndices(GEle)
                    );
            }
        }
        if(isStoreFromStruct(SI)){
            generateWarning(SI, "is Store from struct");
            GetElementPtrInst * GEle = getStoredStructEle(SI);
            if(GEle != NULL){
            //     FEle->addVariable(
            //             getLoadeeValue(GEle->getPointerOperand()),
            //             GEle->getResultElementType(),
            //             GEle->getSourceElementType(),
            //             getValueIndices(GEle)
            //         );
            //     FEle->incrementRefCount(
            //             getLoadeeValue(GEle->getPointerOperand()),
            //             GEle->getResultElementType(),
            //             getValueIndices(GEle),
            //             SI->getPointerOperand()
            //         );
            }
        }
    }
     
    void Analyzer::analyzeCallInst(CallInst *CI, BasicBlock &B){
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
            FEle->setCorrectlyBranched(&B);
        }
    }
    
    void Analyzer::checkAvailability() {
        FreedStructList fsl = FEle->getFreedStruct();

        // for(FreedStruct * localVar: FEle->getLocalVar()) {
        //     if(!FEle->isArgValue(localVar->getValue())){
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
            int ind = 0;
            for (Type * t: strTy->elements()) {
                if (!t->isPointerTy() 
                        || isFuncPointer(t)
                        || alreadyFreed[ind]){
                    cPointers--;
                } else {
                    ValueInformation *vinfo = FEle->getValueInfo(freedStruct->getValue(), t, ind);
                    if(vinfo != NULL){
                        if(FEle->isFreedInBasicBlock(freedStruct->getFreedBlock(), vinfo->getValue(), t, ind)
                                || FEle->isCorrectlyBranchedFreeValue(freedStruct->getFreedBlock(), vinfo->getValue(), t, ind))
                            cPointers--;
                        else if (!vinfo->noRefCount()) {
                            for(Value * referee: vinfo->getReferees()) {
                                if (FEle->isLiveInBasicBlock(freedStruct->getFreedBlock(), referee)){
                                    cPointers--;
                                    break;
                                }
                            }
                        }

                        FEle->setStructMemberFreed(freedStruct, vinfo->getMemberNum());
                        if(FEle->isArgValue(vinfo->getValue())) {
                            FEle->setStructMemberArgFreed(vinfo->getValue(), vinfo->getMemberNum());
                            cPointers--;
                        }
                    }
                }
                ind++;
            }
            if (!FEle->isArgValue(freedStruct->getValue())){
                // if(cPointers > 0)
                //     generateError(freedStruct->getInst(), "Struct element is NOT Freed");
                stManage->addCandidateValue(&(FEle->getFunction()), strTy, freedStruct);
            }
        }
        // FEle->printVal();
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

    bool Analyzer::isLoopBlock(BasicBlock& B){
        if(string(B.getName()).find("for.body") != string::npos)
            return true;
        if(string(B.getName()).find("while.body") != string::npos)
            return true;
    }

//     void Analyzer::addLocalStruct(BasicBlock *B, Type * T, Value * V, Instruction * I, ParentList P){
//         ValueInformation *vinfo = FEle->addVariable(V);
//         FEle->addBasicBlockLiveVariable(B, V);
//         if (StructType * strTy = dyn_cast<StructType>(get_type(T))) {
//             FEle->addLocalVar(B, strTy, V, I, P, vinfo);

//             P.push_back(T);
//             for (Type * ele: strTy->elements()) {
//                 if(ele->isStructTy())
//                     this->addLocalStruct(B, ele, V, I, P);
//             }
// 		}
//         return;
//     }

    void Analyzer::addLocalVariable(BasicBlock *B, Type * T, Value * V, Instruction * I, ParentList P){
        ValueInformation *vinfo = FEle->addVariable(V);
        FEle->addBasicBlockLiveVariable(B, V);
        if (StructType * strTy = dyn_cast<StructType>(T)) {
            FEle->addLocalVar(B, strTy, V, I, P, vinfo);

            P.push_back(T);
            for (Type * ele: strTy->elements()) {
                if(ele->isStructTy())
                    this->addLocalVariable(B, ele, V, I, P);
            }
		}
        return;
    }

    void Analyzer::addPointerLocalVariable(BasicBlock *B, Type * T, Value * V, Instruction * I, ParentList P){
        ValueInformation *vinfo = FEle->addVariable(V);
        if (StructType * strTy = dyn_cast<StructType>(get_type(T))) {
            FEle->addLocalVar(B, strTy, V, I, P, vinfo);

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
                        FEle->addFreeValue(
                                B,
                                loaded_value,
                                getStructType(val),
                                NULL,
                                -1);
                        FEle->addFreedStruct(B, getStructType(val), loaded_value, val);
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

                // FEle->addAllocValue(
                //         B,
                //         getLoadeeValue(inst->getPointerOperand()),
                //         inst->getResultElementType()
                //         );
                generateWarning(CI, "Struct element malloc");
            }
        } else {
            Value * val = getAllocatedValue(CI);
            if (FEle->isArgValue(val))
                FEle->setArgAlloc(val);
            this->addPointerLocalVariable(B, val->getType(), val, CI, ParentList());
            // FEle->addAllocValue(B, val, val->getType());
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
