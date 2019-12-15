#include "include/StageOneAnalyzer.hpp"

namespace ST_free {
    
    void StageOneAnalyzer::analyzeInstructions(BasicBlock &B) {
        for (Instruction &I: B) {
            if(this->isReturnFunc(&I))
                getFunctionInformation()->addEndPoint(&B);

            if (InstAnalysisMap.find(I.getOpcode()) != InstAnalysisMap.end())
                (this->*InstAnalysisMap[I.getOpcode()])(&I, B);
        }
    }

    void StageOneAnalyzer::analyzeAllocaInst(Instruction* AI, BasicBlock &B){
    }

    void StageOneAnalyzer::analyzeStoreInst(Instruction *I, BasicBlock &B){
        StoreInst *SI = cast<StoreInst>(I);
        AliasElement valueEle, pointerEle;

        if (CallInst* CI = this->getStoreFromCall(SI))
            this->getFunctionInformation()->getBasicBlockInformation(&B)->addStoredCallValues(SI->getPointerOperand(), CI);

        /*** Check the Pointer of StoreInst ***/
        if(this->isStoreToStructMember(SI)) {
            // generateWarning(SI, "is Store to struct member", true);
            GetElementPtrInst * GEle = getStoredStruct(SI);
            if(GEle != NULL && isa<StructType>(GEle->getSourceElementType())) {
                generateWarning(SI, "is Store to struct member");
                getStructManager()->addStore(cast<StructType>(GEle->getSourceElementType()), getValueIndices(GEle).back());
                pointerEle.set(cast<StructType>(GEle->getSourceElementType()), getValueIndices(GEle).back());

                // if (auto BCI = dyn_cast<BitCastInst>(SI->getValueOperand())) {
                //     ParentList indexes;
                //     generateWarning(SI, "is Casted Store");
                //     this->getStructParents(GEle, indexes);
                //     if (this->isAuthorityChained(indexes)
                //             && !this->isAllocCast(BCI)) {
                //         generateWarning(SI, "Single Chained", true);
                //         // Do something
                //     }
                // }

                if(GlobalVariable *GV = dyn_cast<GlobalVariable>(SI->getValueOperand())) {
                    generateWarning(SI, "GlobalVariable Store");
                    getStructManager()->addGlobalVarStore(
                            cast<StructType>(GEle->getSourceElementType()), 
                            getValueIndices(GEle).back());
                    // if(GV->getValueType()->isStructTy() && GV->hasInitializer()) {
                    //     if(const DebugLoc &Loc = SI->getDebugLoc()){
                    //         vector<string> dirs = this->decodeDirectoryName(string(Loc->getFilename()));
                    //         getStructManager()->get(cast<StructType>(GEle->getSourceElementType()))->addGVInfo(getValueIndices(GEle), dirs, GV);
                    //     }
                    // }
                }

                if(LoadInst *LI = dyn_cast<LoadInst>(SI->getValueOperand())) {
                    if(isa<AllocaInst>(LI->getPointerOperand())) {
                        getFunctionInformation()->setAlias(GEle, LI->getPointerOperand());
                    }
                }
            }
        } else if(this->isStoreToStruct(SI)) {
            generateWarning(SI, "is Store To Struct");
        }

        /*** Check the Value of the StoreInst ***/
        if(this->isStoreFromStructMember(SI)) {
            generateWarning(SI, "is Store from struct member");
            GetElementPtrInst * GEle = getStoredStructEle(SI);
            if(GEle != NULL && isa<AllocaInst>(SI->getPointerOperand())) {
                getFunctionInformation()->setAlias(GEle, SI->getPointerOperand());
            }
        } else if(this->isStoreFromStruct(SI)) {
            generateWarning(SI, "is Store from struct");
            valueEle.set(cast<StructType>(get_type(SI->getValueOperand()->getType())), ROOT_INDEX);
        }

        if(valueEle.stTy != NULL && pointerEle.stTy != NULL) {
            generateWarning(SI, "Add to Relationship Manager");
            getTypeRelationManager()->add(valueEle, pointerEle);
        }

        // if(get_type(SI->getValueOperand()->getType())->isFunctionTy()) {
        //     generateWarning(SI, "is Function Type");
        //     getFunctionInformation()->addFunctionPointerInfo(SI->getPointerOperand(), cast<Function>(SI->getValueOperand()));
        // }
    }
     
    void StageOneAnalyzer::analyzeCallInst(Instruction *I, BasicBlock &B) {
        CallInst *CI = cast<CallInst>(I);

        vector<Function *> funcLists;
        if (CI->isIndirectCall()) {
            if (LoadInst *LI = dyn_cast<LoadInst>(CI->getCalledValue())) {
                vector<pair<Type *, int>> typeList;

                funcLists = getFunctionInformation()->getPointedFunctions(LI->getPointerOperand());
                if(const DebugLoc &Loc = CI->getDebugLoc()){
                    vector<string> dirs = this->decodeDirectoryName(string(Loc->getFilename()));
                    this->getStructParents(LI, typeList);
                    // if(typeList.size() > 0){
                    //     cast<StructType>(typeList[0].first);
                    //     vector<globalVarInfo> gvi = getStructManager()->get(cast<StructType>(typeList[0].first))->getGVInfo(typeList[0].second);
                    //     for(globalVarInfo gv: gvi) {
                    //         // Do something
                    //     }
                    // }
                }
            }
        }

        if (Function* called_function = CI->getCalledFunction()) {
            funcLists.push_back(called_function);
        }

        for(Function* called_function: funcLists) {
            if (isAllocFunction(called_function)) {
                this->addAlloc(CI, &B);
            } else if (isFreeFunction(called_function)) {
                for (auto arguments = CI->arg_begin(); arguments != CI->arg_end(); arguments++) {
                    this->addFree(cast<Value>(arguments), CI, &B);
                }
            } else {
                this->analyzeDifferentFunc((Function &)(*called_function));
                this->copyArgStatus((Function &)(*called_function), CI, B);
                this->copyAllocatedStatus((Function &)(*called_function), B);
            }
        }
    }

    void StageOneAnalyzer::analyzeBranchInst(Instruction* I, BasicBlock &B) {
        BranchInst *BI = cast<BranchInst>(I);
        if (BI->isConditional()) {
            if (auto ICI = dyn_cast<ICmpInst>(BI->getCondition())) {
                int op = this->getErrorOperand(ICI);
                int errcode = 0;

                if (op >= 0) {
                    BasicBlock *errBlock = BI->getSuccessor(op);
                    for(auto ele: this->getErrorValues(ICI, B, errcode).getList()) {
                        this->getFunctionInformation()
                            ->getBasicBlockInformation(&B)
                            ->addRemoveAlloc(errBlock, const_cast<UniqueKey *>(ele));
                    }
                }
            }
        }

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

                // Add to Allocated
                if (getFunctionInformation()->isAllocatedInBasicBlock(freedStruct->getFreedBlock(), NULL, t, ROOT_INDEX)) {
                    generateWarning(freedStruct->getInst(), "Allocated");
                    getFunctionInformation()->setStructMemberAllocated(freedStruct, ind);
                }

                ValueInformation *vinfo = getFunctionInformation()->getValueInfo(freedStruct->getValue(), t, ind);
                if (vinfo != NULL) {
                    if (getFunctionInformation()->isFreedInBasicBlock(freedStruct->getFreedBlock(), vinfo->getValue(), t, ind)
                            || getFunctionInformation()->isCorrectlyBranchedFreeValue(freedStruct->getFreedBlock(), vinfo->getValue(), t, ind)
                        ) {
                        getFunctionInformation()->setStructMemberFreed(freedStruct, vinfo->getMemberNum());
                        if (getFunctionInformation()->isArgValue(vinfo->getValue())) {
                            getFunctionInformation()->setStructMemberArgFreed(vinfo->getValue(), vinfo->getParents());
                        }
                    }
                }
            }

            if (freedStruct->isInStruct() || !getFunctionInformation()->isArgValue(freedStruct->getValue())) {
                getStructManager()->addCandidateValue(&(getFunctionInformation()->getFunction()), strTy, freedStruct);
            }
        }
        return;
    }
}
