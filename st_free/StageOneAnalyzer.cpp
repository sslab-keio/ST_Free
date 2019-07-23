#include "include/StageOneAnalyzer.hpp"

namespace ST_free {
    
    void StageOneAnalyzer::analyzeInstructions(BasicBlock &B) {
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

    void StageOneAnalyzer::analyzeAllocaInst(AllocaInst * AI, BasicBlock &B){
        // this->addLocalVariable(
        //         &B,
        //         ainst->getAllocatedType(),
        //         ainst,
        //         cast<Instruction>(getFirstUser(&I)),
        //         ParentList()
        //     );
    }

    void StageOneAnalyzer::analyzeStoreInst(StoreInst * SI, BasicBlock &B){
        AliasElement valueEle, pointerEle;

        /*** Check the Pointer of StoreInst ***/
        if(this->isStoreToStructMember(SI)){
            generateWarning(SI, "is Store to struct member");
            GetElementPtrInst * GEle = getStoredStruct(SI);
            getStructManager()->addStore(cast<StructType>(GEle->getSourceElementType()), getValueIndices(GEle));
            pointerEle.set(cast<StructType>(GEle->getSourceElementType()), getValueIndices(GEle));

            if(isa<GlobalValue>(SI->getValueOperand())) {
                generateWarning(SI, "GolbalVariable Store");
                getStructManager()->addGlobalVarStore(
                        cast<StructType>(GEle->getSourceElementType()), 
                        getValueIndices(GEle)
                    );
            }

            if(LoadInst *LI = dyn_cast<LoadInst>(SI->getValueOperand())) {
                if(isa<AllocaInst>(LI->getPointerOperand())) {
                    getFunctionInformation()->setAliasInBasicBlock(&B, GEle, LI->getPointerOperand());
                }
            }
        } else if(this->isStoreToStruct(SI)) {
            generateWarning(SI, "is Store To Struct");
        }

        /*** Check the Value of the StoreInst ***/
        if(this->isStoreFromStructMember(SI)) {
            generateWarning(SI, "is Store from struct member");
            GetElementPtrInst * GEle = getStoredStructEle(SI);
            if(isa<AllocaInst>(SI->getPointerOperand())) {
                getFunctionInformation()->setAliasInBasicBlock(&B, GEle, SI->getPointerOperand());
            }
        } else if(this->isStoreFromStruct(SI)) {
            generateWarning(SI, "is Store from struct");
            valueEle.set(cast<StructType>(get_type(SI->getValueOperand()->getType())), ROOT_INDEX);
        }

        if(valueEle.stTy != NULL && pointerEle.stTy != NULL) {
            generateWarning(SI, "Add to Relationship Manager");
            getTypeRelationManager()->add(valueEle, pointerEle);
        }
    }
     
    void StageOneAnalyzer::analyzeCallInst(CallInst *CI, BasicBlock &B) {
        if (Function* called_function = CI->getCalledFunction()) {
            if (isAllocFunction(called_function)) {
                Value * val = getAllocatedValue(CI);
                if(val != NULL) 
                    if(StructType * strTy = dyn_cast<StructType>(get_type(val->getType()))) {
                        getStructManager()->addAlloc(strTy);
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

    void StageOneAnalyzer::analyzeBranchInst(BranchInst * BI, BasicBlock &B){
        if(this->isCorrectlyBranched(BI)) {
            generateWarning(BI, "Correctly Branched");
            getFunctionInformation()->setCorrectlyBranched(&B);
        }
    }
    
    void StageOneAnalyzer::checkAvailability() {
        FreedStructList fsl = getFunctionInformation()->getFreedStruct();

        for(FreedStruct * freedStruct: fsl) {
            StructType * strTy = cast<StructType>(freedStruct->getType());
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
                getStructManager()->addCandidateValue(&(getFunctionInformation()->getFunction()), strTy, freedStruct);
            }
        }
        return;
    }
}
