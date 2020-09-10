#include "include/StageOneAnalyzer.hpp"

namespace ST_free {

void StageOneAnalyzer::analyzeInstructions(llvm::BasicBlock &B) {
  for (llvm::Instruction &I : B) {
    if (InstAnalysisMap.find(I.getOpcode()) != InstAnalysisMap.end())
      (this->*InstAnalysisMap[I.getOpcode()])(&I, B);
  }
}

void StageOneAnalyzer::analyzeAllocaInst(llvm::Instruction *AI,
                                         llvm::BasicBlock &B) {}

void StageOneAnalyzer::analyzeStoreInst(llvm::Instruction *I,
                                        llvm::BasicBlock &B) {
  llvm::StoreInst *SI = llvm::cast<llvm::StoreInst>(I);
  AliasElement valueEle, pointerEle;

  if (llvm::CallInst *CI = this->getStoreFromCall(SI))
    this->getFunctionInformation()
        ->getBasicBlockInformation(&B)
        ->addStoredCallValues(SI->getPointerOperand(), CI);

  /*** Check the Pointer of StoreInst ***/
  if (this->isStoreToStructMember(SI)) {
    generateWarning(SI, "is Store to struct member", true);
    if (llvm::GetElementPtrInst *GEle = getStoredStruct(SI)) {
      generateWarning(SI, "found GetElementPtrInst", true);
      struct collectedInfo info;
      ParentList plist;

      this->collectStructMemberFreeInfo(GEle, info, plist);

      if (info.indexes.size() > 0 &&
          llvm::isa<llvm::StructType>(GEle->getSourceElementType())) {
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

      llvm::Value *addVal = SI->getValueOperand();
      if (llvm::LoadInst *LI = llvm::dyn_cast<llvm::LoadInst>(addVal)) {
        generateWarning(SI, "found load", true);
        if (llvm::isa<llvm::AllocaInst>(LI->getPointerOperand()))
          addVal = LI->getPointerOperand();
      }

      if (addVal && info.indexes.size() > 0) {
        getFunctionInformation()->setAlias(GEle, addVal);
        if (auto StTy = llvm::dyn_cast<llvm::StructType>(
                get_type(info.indexes.back().first))) {
          if (ROOT_INDEX < info.indexes.back().second &&
              info.indexes.back().second < StTy->getNumElements()) {
            generateWarning(I, "[Before] Looking for alloc alias", true);
            if (StTy->hasName()) generateWarning(I, StTy->getName(), true);
            generateWarning(I, std::to_string(info.indexes.back().second),
                            true);
            if (const UniqueKey *src_uk =
                    this->getFunctionInformation()
                        ->getBasicBlockInformation(&B)
                        ->getWorkList(ALLOCATED)
                        .getFromType(
                            StTy->getElementType(info.indexes.back().second))) {
              generateWarning(I, "[After] Found alloc alias", true);
              const UniqueKey *dest_uk =
                  getFunctionInformation()->addAllocValue(
                      &B, NULL,
                      StTy->getElementType(info.indexes.back().second),
                      info.indexes.back().second);

              if (getFunctionInformation()->checkAndPopPendingAliasedAlloc(
                      src_uk)) {
                generateWarning(
                    I, "[After] Found alloc alias in pendling aliased alloc",
                    true);
                getFunctionInformation()->setUniqueKeyAlias(dest_uk, src_uk);
              }
            } else if (isDirectStoreFromAlloc(SI)) {
              generateWarning(I, "[After] Found alloc alias as direct store",
                              true);
              const UniqueKey *dest_uk =
                  getFunctionInformation()->addAllocValue(
                      &B, NULL,
                      StTy->getElementType(info.indexes.back().second),
                      info.indexes.back().second);
            } else {
              if (auto CastI = llvm::dyn_cast<llvm::CastInst>(addVal)) {
                addVal = CastI->getOperand(0);
              }
              if (auto GEleI =
                      llvm::dyn_cast<llvm::GetElementPtrInst>(addVal)) {
                GEleI = this->getRootGEle(GEleI);
                addVal = GEleI->getOperand(0);
              }
              if (getFunctionInformation()->isArgValue(addVal)) {
                getFunctionInformation()->addPendingArgAlloc(
                    &B, NULL, StTy->getElementType(info.indexes.back().second),
                    info.indexes.back().second);
              }
            }
          }
        }
      }
    }
  } else if (this->isStoreToStruct(SI)) {
    if (llvm::StructType *StTy = this->getStoreeStruct(SI)) {
      generateWarning(SI, "is Store To Struct", true);
      if (llvm::StructType *SrcTy = this->getStorerStruct(SI)) {
        if (StTy != SrcTy && get_type(StTy->getElementType(0)) == SrcTy) {
          if (const UniqueKey *src_uk =
                  this->getFunctionInformation()
                      ->getBasicBlockInformation(&B)
                      ->getWorkList(ALLOCATED)
                      .getFromType(StTy->getElementType(0))) {
            const UniqueKey *tgt_uk = getFunctionInformation()->addAllocValue(
                &B, NULL, StTy->getElementType(0), 0);

            // if
            // (getFunctionInformation()->checkAndPopPendingAliasedAlloc(src_uk))
            // {
            //   generateWarning(I, "[After] Found alloc alias in pendling
            //   aliased alloc", true);
            //   getFunctionInformation()->setUniqueKeyAlias(src_uk, tgt_uk);
            // }
          }
        }
      }
    }
  }

  /*** Check the Value of the StoreInst ***/
  if (this->isStoreFromStructMember(SI)) {
    generateWarning(SI, "is Store from struct member");
    llvm::GetElementPtrInst *GEle = getStoredStructEle(SI);
    if (GEle != NULL && llvm::isa<llvm::AllocaInst>(SI->getPointerOperand())) {
      getFunctionInformation()->setAlias(GEle, SI->getPointerOperand());
    }
  } else if (this->isStoreFromStruct(SI)) {
    generateWarning(SI, "is Store from struct", true);
    valueEle.set(llvm::cast<llvm::StructType>(
                     get_type(SI->getValueOperand()->getType())),
                 ROOT_INDEX);
    if (llvm::StructType *SrcTy = this->getStorerStruct(SI)) {
      generateWarning(SI, "Found Storer Struct", true);
    }
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

void StageOneAnalyzer::analyzeCallInst(llvm::Instruction *I,
                                       llvm::BasicBlock &B) {
  llvm::CallInst *CI = llvm::cast<llvm::CallInst>(I);

  std::vector<llvm::Function *> funcLists;

  // Support indirect calls
  if (CI->isIndirectCall()) {
    if (llvm::LoadInst *LI =
            llvm::dyn_cast<llvm::LoadInst>(CI->getCalledValue())) {
      generateWarning(CI, "Found Indirect Called Function");
      std::vector<std::pair<llvm::Type *, int>> typeList;

      funcLists = getFunctionInformation()->getPointedFunctions(
          LI->getPointerOperand());
      if (const llvm::DebugLoc &Loc = CI->getDebugLoc()) {
        std::string path = Loc->getFilename();
        this->getStructParents(LI, typeList);
        if (typeList.size() > 0) {
          generateWarning(CI, "Indirect call using struct member", true);
          if (auto parent_type = llvm::dyn_cast<llvm::StructType>(
                  get_type(typeList.back().first))) {
            if (getStructManager()->exists(parent_type)) {
              for (llvm::Function *called_function:
                   getStructManager()
                       ->get(parent_type)
                       ->getFunctionPtr(typeList.back().second)) {
                generateWarning(CI, "Found indirect call candidate", true);
                funcLists.push_back(called_function);
              }
            }
          }
        }
      }
    }
  }

  // Normal calls
  if (llvm::Function *called_function = CI->getCalledFunction()) {
    funcLists.push_back(called_function);
  }

  // Try checking bitcast included calls
  if (funcLists.empty()) {
    if (auto *ConstExpr =
            llvm::dyn_cast<llvm::ConstantExpr>(CI->getCalledOperand())) {
      if (ConstExpr->isCast()) {
        if (auto *func =
                llvm::dyn_cast<llvm::Function>(ConstExpr->getOperand(0))) {
          funcLists.push_back(func);
        }
      }
    }
  }

  for (llvm::Function *called_function : funcLists) {
    if (called_function && called_function->hasName())
      generateWarning(CI, called_function->getName(), true);

    if (isAllocFunction(called_function)) {
      this->addAlloc(CI, &B);
    } else if (isFreeFunction(called_function)) {
      for (auto arguments = CI->arg_begin(); arguments != CI->arg_end();
           arguments++) {
        this->addFree(llvm::cast<llvm::Value>(arguments), CI, &B);
      }
    } else if (isSpecializedFreeFunction(called_function)) {
      for (auto arguments = CI->arg_begin(); arguments != CI->arg_end();
           arguments++) {
        this->addRefcountedFree(llvm::cast<llvm::Value>(arguments), CI, &B);
      }
    } else {
      this->analyzeDifferentFunc((llvm::Function &)(*called_function));
      generateWarning(CI, "Copy all the status", true);
      this->copyAllocatedStatus((llvm::Function &)(*called_function), CI, B);
      this->copyFreeStatus((llvm::Function &)(*called_function), CI, B);
      this->evaluatePendingStoredValue((llvm::Function &)(*called_function), CI,
                                       B);
    }
  }
}

void StageOneAnalyzer::analyzeBranchInst(llvm::Instruction *I,
                                         llvm::BasicBlock &B) {
  llvm::BranchInst *BI = llvm::cast<llvm::BranchInst>(I);
  if (BI->isConditional()) {
    generateWarning(I, "Calling is conditional");
    if (auto ICI = llvm::dyn_cast<llvm::ICmpInst>(BI->getCondition())) {
      if (this->isCallInstReturnValue(ICI->getOperand(0))) {
        this->analyzeErrorCode(BI, ICI, B);
      }

      if (llvm::isa<llvm::ConstantPointerNull>(ICI->getOperand(1))) {
        this->analyzeNullCheck(BI, ICI, B);
      }
    } else if (llvm::CallInst *CI =
                   llvm::dyn_cast<llvm::CallInst>(BI->getCondition())) {
      if (CI->getCalledFunction() && isIsErrFunction(CI->getCalledFunction())) {
        this->analyzeErrorCheckFunction(BI, CI, B);
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

void StageOneAnalyzer::analyzeGetElementPtrInst(llvm::Instruction *I,
                                                llvm::BasicBlock &B) {
  llvm::GetElementPtrInst *GEleI = llvm::cast<llvm::GetElementPtrInst>(I);
  struct collectedInfo info;
  ParentList plist;

  // generateWarning(GEleI, "found GetElementPtrInst", true);
  this->collectStructMemberFreeInfo(GEleI, info, plist);

  if (info.indexes.size() > 0) {
    if (auto StTy = llvm::dyn_cast<llvm::StructType>(
            get_type(info.indexes.back().first))) {
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
    llvm::StructType *strTy =
        llvm::cast<llvm::StructType>(freedStruct->getType());
    std::vector<bool> alreadyFreed = freedStruct->getFreedMember();

    for (int ind = 0; ind < strTy->getNumElements(); ind++) {
      llvm::Type *t = strTy->getElementType(ind);
      if (isFuncPointer(t) || alreadyFreed[ind]) continue;

      // Add to Allocated
      if (getFunctionInformation()->isAllocatedInBasicBlock(
              freedStruct->getFreedBlock(), NULL, t, ind)) {
        generateWarning(freedStruct->getInst(),
                        "[INDEXED VERSION] Allocated(second) + ind :" +
                            std::to_string(ind));
        getFunctionInformation()->setStructMemberAllocated(freedStruct, ind);
      }

      if (getFunctionInformation()->isFreedInBasicBlock(
              freedStruct->getFreedBlock(), NULL, t, ind) ||
          getFunctionInformation()->isCorrectlyBranchedFreeValue(
              freedStruct->getFreedBlock(), NULL, t, ind)) {
        getFunctionInformation()->setStructMemberFreed(freedStruct, ind);
      }
    }

    // if (freedStruct->isInStruct() ||
    //     !getFunctionInformation()->isArgValue(freedStruct->getValue())) {
    getStructManager()->addCandidateValue(
        &(getFunctionInformation()->getFunction()), strTy, freedStruct);
    // }
  }
  return;
}
}  // namespace ST_free
