#include "include/StageOneAnalyzer.hpp"

namespace ST_free {

void StageOneAnalyzer::analyzeInstructions(BasicBlock &B) {
  for (Instruction &I : B) {
    if (this->isReturnFunc(&I)) {
      getFunctionInformation()->addEndPoint(&B);
    }

    if (InstAnalysisMap.find(I.getOpcode()) != InstAnalysisMap.end())
      (this->*InstAnalysisMap[I.getOpcode()])(&I, B);
  }
}

void StageOneAnalyzer::analyzeAllocaInst(Instruction *AI, BasicBlock &B) {}

void StageOneAnalyzer::analyzeStoreInst(Instruction *I, BasicBlock &B) {
  StoreInst *SI = cast<StoreInst>(I);
  AliasElement valueEle, pointerEle;

  if (CallInst *CI = this->getStoreFromCall(SI))
    this->getFunctionInformation()
        ->getBasicBlockInformation(&B)
        ->addStoredCallValues(SI->getPointerOperand(), CI);

  /*** Check the Pointer of StoreInst ***/
  if (this->isStoreToStructMember(SI)) {
    generateWarning(SI, "is Store to struct member");
    if (GetElementPtrInst *GEle = getStoredStruct(SI)) {
      generateWarning(SI, "found GetElementPtrInst");
      struct collectedInfo info;
      ParentList plist;

      this->collectStructMemberFreeInfo(GEle, info, plist);

      if (info.indexes.size() > 0 &&
          isa<StructType>(GEle->getSourceElementType())) {
        // getStructManager()->addStore(cast<StructType>(GEle->getSourceElementType()),
        // getValueIndices(GEle).back());
        // pointerEle.set(cast<StructType>(GEle->getSourceElementType()),
        // getValueIndices(GEle).back());

        // if(GlobalVariable *GV =
        // dyn_cast<GlobalVariable>(SI->getValueOperand())) {
        //     generateWarning(SI, "GlobalVariable Store");
        //     getStructManager()->addGlobalVarStore(
        //             cast<StructType>(GEle->getSourceElementType()),
        //             getValueIndices(GEle).back());
        //     // if(GV->getValueType()->isStructTy() && GV->hasInitializer()) {
        //     //     if(const DebugLoc &Loc = SI->getDebugLoc()){
        //     //         vector<string> dirs =
        //     this->decodeDirectoryName(string(Loc->getFilename()));
        //     //
        //     getStructManager()->get(cast<StructType>(GEle->getSourceElementType()))->addGVInfo(getValueIndices(GEle),
        //     dirs, GV);
        //     //     }
        //     // }
        // }
      }

      Value *addVal = SI->getValueOperand();
      if (LoadInst *LI = dyn_cast<LoadInst>(addVal)) {
        generateWarning(SI, "is Store to struct member");
        if (isa<AllocaInst>(LI->getPointerOperand()))
          addVal = LI->getPointerOperand();
      }

      if (addVal) {
        getFunctionInformation()->setAlias(GEle, addVal);
        if (info.indexes.size() > 0) {
          if (auto StTy =
                  dyn_cast<StructType>(get_type(info.indexes.back().first))) {
            if (ROOT_INDEX < info.indexes.back().second &&
                info.indexes.back().second < StTy->getNumElements()) {
              generateWarning(I, "[Before] Looking for alloc alias");
              if (const UniqueKey* src_uk = this->getFunctionInformation()
                      ->getBasicBlockInformation(&B)
                      ->getWorkList(ALLOCATED)
                      .getFromType(
                          StTy->getElementType(info.indexes.back().second))) {
                generateWarning(I, "[After] Found alloc alias");
                const UniqueKey* dest_uk = getFunctionInformation()->addAllocValue(
                    &B, NULL, StTy->getElementType(info.indexes.back().second),
                    info.indexes.back().second);

                getFunctionInformation()->setUniqueKeyAlias(src_uk, dest_uk);
              } else {
                if (auto CastI = dyn_cast<CastInst>(addVal)) {
                  addVal = CastI->getOperand(0);
                }
                if (auto GEleI = dyn_cast<GetElementPtrInst>(addVal)) {
                  GEleI = this->getRootGEle(GEleI);
                  addVal = GEleI->getOperand(0);
                }
                if (getFunctionInformation()->isArgValue(addVal)) {
                  getFunctionInformation()->addPendingArgAlloc(
                      &B, NULL,
                      StTy->getElementType(info.indexes.back().second),
                      info.indexes.back().second);
                }
              }
            }
          }
        }
      }
    }
  } else if (this->isStoreToStruct(SI)) {
    if (StructType *StTy = this->getStoreeStruct(SI)) {
      generateWarning(SI, "is Store To Struct");
      if (StructType *SrcTy = this->getStorerStruct(SI)) {
        if (StTy != SrcTy && get_type(StTy->getElementType(0)) == SrcTy) {
          if (const UniqueKey* src_uk = this->getFunctionInformation()
                  ->getBasicBlockInformation(&B)
                  ->getWorkList(ALLOCATED)
                  .getFromType(StTy->getElementType(0))) {
            const UniqueKey* tgt_uk = getFunctionInformation()->addAllocValue(&B, NULL,
                                                    StTy->getElementType(0), 0);
            getFunctionInformation()->setUniqueKeyAlias(src_uk, tgt_uk);
          }
        }
      }
    }
  }

  /*** Check the Value of the StoreInst ***/
  if (this->isStoreFromStructMember(SI)) {
    generateWarning(SI, "is Store from struct member");
    GetElementPtrInst *GEle = getStoredStructEle(SI);
    if (GEle != NULL && isa<AllocaInst>(SI->getPointerOperand())) {
      getFunctionInformation()->setAlias(GEle, SI->getPointerOperand());
    }
  } else if (this->isStoreFromStruct(SI)) {
    generateWarning(SI, "is Store from struct");
    valueEle.set(cast<StructType>(get_type(SI->getValueOperand()->getType())),
                 ROOT_INDEX);
  }

  if (valueEle.stTy != NULL && pointerEle.stTy != NULL) {
    generateWarning(SI, "Add to Relationship Manager");
    getTypeRelationManager()->add(valueEle, pointerEle);
  }

  // if(get_type(SI->getValueOperand()->getType())->isFunctionTy()) {
  //     generateWarning(SI, "is Function Type");
  //     getFunctionInformation()->addFunctionPointerInfo(SI->getPointerOperand(),
  //     cast<Function>(SI->getValueOperand()));
  // }
}

void StageOneAnalyzer::analyzeCallInst(Instruction *I, BasicBlock &B) {
  CallInst *CI = cast<CallInst>(I);

  vector<Function *> funcLists;
  if (CI->isIndirectCall()) {
    if (LoadInst *LI = dyn_cast<LoadInst>(CI->getCalledValue())) {
      vector<pair<Type *, int>> typeList;

      funcLists = getFunctionInformation()->getPointedFunctions(
          LI->getPointerOperand());
      if (const DebugLoc &Loc = CI->getDebugLoc()) {
        vector<string> dirs =
            this->decodeDirectoryName(string(Loc->getFilename()));
        this->getStructParents(LI, typeList);
        // if(typeList.size() > 0){
        //     cast<StructType>(typeList[0].first);
        //     vector<globalVarInfo> gvi =
        //     getStructManager()->get(cast<StructType>(typeList[0].first))->getGVInfo(typeList[0].second);
        //     for(globalVarInfo gv: gvi) {
        //         // Do something
        //     }
        // }
      }
    }
  }

  if (Function *called_function = CI->getCalledFunction()) {
    funcLists.push_back(called_function);
  }

  for (Function *called_function : funcLists) {
    if (isAllocFunction(called_function)) {
      this->addAlloc(CI, &B);
    } else if (isFreeFunction(called_function)) {
      for (auto arguments = CI->arg_begin(); arguments != CI->arg_end();
           arguments++) {
        this->addFree(cast<Value>(arguments), CI, &B);
      }
    } else {
      this->analyzeDifferentFunc((Function &)(*called_function));
      this->copyAllocatedStatus((Function &)(*called_function), B);
      this->copyFreeStatus((Function &)(*called_function), CI, B);
      this->evaluatePendingStoredValue((Function &)(*called_function), CI, B);
    }
  }
}

void StageOneAnalyzer::analyzeBranchInst(Instruction *I, BasicBlock &B) {
  BranchInst *BI = cast<BranchInst>(I);
  if (BI->isConditional()) {
    generateWarning(I, "Calling is conditional");
    if (auto ICI = dyn_cast<ICmpInst>(BI->getCondition())) {
      int op = this->getErrorOperand(ICI);
      int errcode = 0;

      if (op >= 0) {
        BasicBlock *errBlock = BI->getSuccessor(op);
        this->getFunctionInformation()
            ->getBasicBlockInformation(&B)
            ->addSucceedingErrorBlock(errBlock);
        for (auto ele : this->getErrorValues(ICI, B, errcode).getList()) {
          this->getFunctionInformation()
              ->getBasicBlockInformation(&B)
              ->addRemoveAlloc(errBlock, const_cast<UniqueKey *>(ele));
        }
      }
    } else if (CallInst *CI = dyn_cast<CallInst>(BI->getCondition())) {
      if (CI->getCalledFunction() && isIsErrFunction(CI->getCalledFunction())) {
        generateWarning(CI, "Calling IS_ERR()");
        BasicBlock *errBlock = BI->getSuccessor(0);
        this->getFunctionInformation()
            ->getBasicBlockInformation(&B)
            ->addSucceedingErrorBlock(errBlock);

        Value* tgt_val = CI->getArgOperand(0);
        if (auto BCI = dyn_cast<BitCastInst>(tgt_val)) {
          tgt_val = BCI->getOperand(0);
        }
        ParentList plist = this->decodeErrorTypes(tgt_val);
        Type *Ty =
            this->getComparedType(decodeComparedValue(tgt_val), B);
        if (plist.size() > 0) {
          generateWarning(CI, "Calling IS_ERR(): plist");
          if (auto StTy = dyn_cast<StructType>(get_type(plist.back().first))) {
            if (0 <= plist.back().second &&
                plist.back().second < StTy->getNumElements())
              Ty = StTy->getElementType(plist.back().second);
          }
          if (this->getFunctionInformation()->isAllocatedInBasicBlock(
                  &B, NULL, Ty, plist.back().second)) {
          generateWarning(CI, "Calling IS_ERR(): plist found alloc");
          this->getFunctionInformation()
              ->getBasicBlockInformation(&B)
              ->addRemoveAlloc(errBlock,
                               const_cast<UniqueKey *>(
                                   this->getFunctionInformation()
                                       ->getUniqueKeyManager()
                                       ->getUniqueKey(NULL, Ty, plist.back().second)));
          }
        }
        if (this->getFunctionInformation()->isAllocatedInBasicBlock(
                errBlock, NULL, Ty, ROOT_INDEX)) {
          this->getFunctionInformation()
              ->getBasicBlockInformation(&B)
              ->addRemoveAlloc(errBlock,
                               const_cast<UniqueKey *>(
                                   this->getFunctionInformation()
                                       ->getUniqueKeyManager()
                                       ->getUniqueKey(NULL, Ty, ROOT_INDEX)));
        }
      }
    }
  } else {
    generateWarning(BI, "Is unconditional branch");
    this->getFunctionInformation()
        ->getBasicBlockInformation(&B)
        ->setUnconditionalBranched();
  }

  if (this->isCorrectlyBranched(BI)) {
    generateWarning(BI, "Correctly Branched");
    getFunctionInformation()->setCorrectlyBranched(&B);
  }
}

void StageOneAnalyzer::analyzeGetElementPtrInst(Instruction *I, BasicBlock &B) {
  GetElementPtrInst *GEleI = cast<GetElementPtrInst>(I);
  struct collectedInfo info;
  ParentList plist;

  // generateWarning(GEleI, "found GetElementPtrInst", true);
  this->collectStructMemberFreeInfo(GEleI, info, plist);

  if (info.indexes.size() > 0) {
    if (auto StTy = dyn_cast<StructType>(get_type(info.indexes.back().first))) {
      if (ROOT_INDEX < info.indexes.back().second &&
          info.indexes.back().second < StTy->getNumElements()) {
        generateWarning(I, "[Before] Found alloc alias");
      }
    }
  }
  return;
}

void StageOneAnalyzer::checkAvailability() {
  FreedStructList fsl = getFunctionInformation()->getFreedStruct();

  for (FreedStruct *freedStruct : fsl) {
    StructType *strTy = cast<StructType>(freedStruct->getType());
    vector<bool> alreadyFreed = freedStruct->getFreedMember();

    for (int ind = 0; ind < strTy->getNumElements(); ind++) {
      Type *t = strTy->getElementType(ind);
      if (!t->isPointerTy() || isFuncPointer(t) || alreadyFreed[ind]) continue;

      // Add to Allocated
      // if
      // (getFunctionInformation()->isAllocatedInBasicBlock(freedStruct->getFreedBlock(),
      // NULL, t, ROOT_INDEX)) {
      //     generateWarning(freedStruct->getInst(), "[NON VALUE] Allocated +
      //     ind :" + to_string(ind), true);
      //     getFunctionInformation()->setStructMemberAllocated(freedStruct,
      //     ind);
      // }

      // Add to Allocated
      if (getFunctionInformation()->isAllocatedInBasicBlock(
              freedStruct->getFreedBlock(), NULL, t, ind)) {
        generateWarning(
            freedStruct->getInst(),
            "[INDEXED VERSION] Allocated(second) + ind :" + to_string(ind),
            true);
        getFunctionInformation()->setStructMemberAllocated(freedStruct, ind);
      }

      // ValueInformation *vinfo_nulled =
      // getFunctionInformation()->getValueInfo(NULL, t, ind); if (vinfo_nulled
      // != NULL) {
      if (getFunctionInformation()->isFreedInBasicBlock(
              freedStruct->getFreedBlock(), NULL, t, ind) ||
          getFunctionInformation()->isCorrectlyBranchedFreeValue(
              freedStruct->getFreedBlock(), NULL, t, ind)) {
        getFunctionInformation()->setStructMemberFreed(freedStruct, ind);
        // if (getFunctionInformation()->isArgValue(freedStruct->getValue())) {
        //     getFunctionInformation()->setStructMemberArgFreed(vinfo->getValue(),
        //     vinfo->getParents());
        // }
      }
      // }
    }

    if (freedStruct->isInStruct() ||
        !getFunctionInformation()->isArgValue(freedStruct->getValue())) {
      getStructManager()->addCandidateValue(
          &(getFunctionInformation()->getFunction()), strTy, freedStruct);
    }
  }
  return;
}
}  // namespace ST_free
