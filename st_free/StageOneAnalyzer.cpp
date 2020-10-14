#include "include/StageOneAnalyzer.hpp"

namespace ST_free {

void StageOneAnalyzer::analyzeInstructions(llvm::BasicBlock &B) {
  for (llvm::Instruction &I : B) {
    if (InstAnalysisMap.find(I.getOpcode()) != InstAnalysisMap.end())
      (this->*InstAnalysisMap[I.getOpcode()])(&I, B);
  }
}

void StageOneAnalyzer::analyzeAllocaInst(llvm::Instruction *I,
                                         llvm::BasicBlock &B) {
  llvm::AllocaInst *AI = llvm::cast<llvm::AllocaInst>(I);
  if (AI->getAllocatedType()->isStructTy()) {
    getFunctionInformation()->appendLocalVariable(AI);
  }
}

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
    STFREE_LOG(SI, "is Store to struct member");
    if (llvm::GetElementPtrInst *GEle = getStoredStruct(SI)) {
      STFREE_LOG(SI, "found GetElementPtrInst");
      struct collectedInfo info;
      ParentList plist;

      this->collectStructMemberFreeInfo(GEle, info, plist);

      llvm::Value *addVal = SI->getValueOperand();
      if (llvm::LoadInst *LI = llvm::dyn_cast<llvm::LoadInst>(addVal)) {
        STFREE_LOG(SI, "found load");
        if (llvm::isa<llvm::AllocaInst>(LI->getPointerOperand()))
          addVal = LI->getPointerOperand();
      } else if (auto *val_gele =
                     llvm::dyn_cast<llvm::GetElementPtrInst>(addVal)) {
        struct collectedInfo gele_info;
        this->collectStructMemberFreeInfo(val_gele, gele_info, plist);
        if (gele_info.freeValue) {
          addVal = gele_info.freeValue;
          if (auto BCI =
                  llvm::dyn_cast<llvm::BitCastInst>(gele_info.freeValue)) {
            addVal = BCI->getOperand(0);
          }
        }
      }

      if (addVal && info.indexes.size() > 0) {
        STFREE_LOG(SI, "Setting Alias");
        getFunctionInformation()->setAlias(GEle, addVal);
        if (auto StTy = llvm::dyn_cast<llvm::StructType>(
                get_type(info.indexes.back().first))) {
          if (ROOT_INDEX < info.indexes.back().second &&
              info.indexes.back().second < StTy->getNumElements()) {
            STFREE_LOG(I, "[Before] Looking for alloc alias");
            if (const UniqueKey *src_uk =
                    this->getFunctionInformation()
                        ->getBasicBlockInformation(&B)
                        ->getWorkList(ALLOCATED)
                        .getFromType(
                            StTy->getElementType(info.indexes.back().second))) {
              STFREE_LOG(I, "[After] Found alloc alias");
              const UniqueKey *dest_uk =
                  getFunctionInformation()->addAllocValue(
                      &B, NULL,
                      StTy->getElementType(info.indexes.back().second),
                      info.indexes.back().second);

              if (getFunctionInformation()->checkAndPopPendingAliasedAlloc(
                      src_uk)) {
                STFREE_LOG(
                    I, "[After] Found alloc alias in pendling aliased alloc");
                getFunctionInformation()->setUniqueKeyAlias(dest_uk, src_uk);
              }
            } else if (isDirectStoreFromAlloc(SI)) {
              STFREE_LOG(I, "[After] Found alloc alias as direct store");
              getFunctionInformation()->addAllocValue(
                  &B, NULL, StTy->getElementType(info.indexes.back().second),
                  info.indexes.back().second);
            } else {
              // At least, so far, we do not know if the storer is allocated or
              // not.
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

            // Look at the parent and see if that is allocated or not.
            // llvm::GetElementPtrInst *root_gele = this->getRootGEle(GEle);
            // if (info.indexes.size() > 0 &&
            //     !this->getFunctionInformation()
            //          ->getBasicBlockInformation(&B)
            //          ->getWorkList(ALLOCATED)
            //          .typeExists(root_gele->getPointerOperandType()) &&
            //     get_type(root_gele->getPointerOperandType())->isStructTy() &&
            //     !getFunctionInformation()->isArgValue(addVal)) {
            //   STFREE_LOG_ON(SI, "Not allocated nor is an argument val");
            //   getFunctionInformation()->addLocalVar(
            //       &B, get_type(root_gele->getPointerOperandType()), addVal,
            //       SI);
            // }
          }
        }
      }
    }
  } else if (this->isStoreToStruct(SI)) {
    if (llvm::StructType *StTy = this->getStoreeStruct(SI)) {
      STFREE_LOG(SI, "is Store To Struct");
      if (llvm::StructType *SrcTy = this->getStorerStruct(SI)) {
        if (StTy != SrcTy && get_type(StTy->getElementType(0)) == SrcTy) {
          if (const UniqueKey *src_uk =
                  this->getFunctionInformation()
                      ->getBasicBlockInformation(&B)
                      ->getWorkList(ALLOCATED)
                      .getFromType(StTy->getElementType(0))) {
            getFunctionInformation()->addAllocValue(&B, NULL,
                                                    StTy->getElementType(0), 0);
          }
        }
      }
    }
  }

  /*** Check the Value of the StoreInst ***/
  if (this->isStoreFromStructMember(SI)) {
    STFREE_LOG(SI, "is Store from struct member");
    // if (info.freeValue) {
    //   llvm::outs() << *info.freeValue << "\n";
    //   // getFunctionInformation()->setAlias(GEle, SI->getPointerOperand());
    // }
  } else if (this->isStoreFromStruct(SI)) {
    STFREE_LOG(SI, "is Store from struct");
    valueEle.set(llvm::cast<llvm::StructType>(
                     get_type(SI->getValueOperand()->getType())),
                 ROOT_INDEX);
    if (llvm::StructType *SrcTy = this->getStorerStruct(SI)) {
      STFREE_LOG(SI, "Found Storer Struct");
    }
  }

  if (valueEle.stTy != NULL && pointerEle.stTy != NULL) {
    STFREE_LOG(SI, "Add to Relationship Manager");
    getTypeRelationManager()->add(valueEle, pointerEle);
  }

  // if(get_type(SI->getValueOperand()->getType())->isFunctionTy()) {
  //     STFREE_LOG(SI, "is Function Type");
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
      STFREE_LOG(CI, "Found Indirect Called Function");
      std::vector<std::pair<llvm::Type *, int>> typeList;
      if (const llvm::DebugLoc &Loc = CI->getDebugLoc()) {
        std::string path = std::string(Loc->getFilename());
        this->getStructParents(LI, typeList);
        if (typeList.size() > 0) {
          STFREE_LOG(CI, "Found Indirect stored struct member");
          if (auto parent_type = llvm::dyn_cast<llvm::StructType>(
                  get_type(typeList.back().first))) {
            if (getStructManager()->exists(parent_type)) {
              STFREE_LOG(CI, "Exists in struct manager");
              if (parent_type->hasName())
                STFREE_LOG(CI, parent_type->getName());
              for (llvm::Function *called_function :
                   getStructManager()
                       ->get(parent_type)
                       ->getFunctionPtr(typeList.back().second, path)) {
                STFREE_LOG(CI, "[INDIRECT]Found indirect call candidate");
                funcLists.push_back(called_function);
              }
            }
          }
        }
      }
    }
  }

  // Normal calls
  if (llvm::Function *called_function = getCalledFunction(CI)) {
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
      STFREE_LOG(CI, called_function->getName());

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
    STFREE_LOG_ON(I, "Calling is conditional");
    if (auto ICI = llvm::dyn_cast<llvm::ICmpInst>(BI->getCondition())) {
      if (this->isCallInstReturnValue(ICI->getOperand(0))) {
        this->analyzeErrorCode(BI, ICI, B);
      }

      if (llvm::isa<llvm::ConstantPointerNull>(ICI->getOperand(1))) {
        this->analyzeNullCheck(BI, ICI, B);
      }
    } else if (llvm::CallInst *CI =
                   llvm::dyn_cast<llvm::CallInst>(BI->getCondition())) {
      if (getCalledFunction(CI) && isIsErrFunction(getCalledFunction(CI))) {
        this->analyzeErrorCheckFunction(BI, CI, B);
      }
    }
  } else {
    STFREE_LOG(BI, "Is unconditional branch");
    this->getFunctionInformation()
        ->getBasicBlockInformation(&B)
        ->setUnconditionalBranched();
  }

  if (this->isCorrectlyBranched(BI)) {
    STFREE_LOG(BI, "Correctly Branched");
    getFunctionInformation()->setCorrectlyBranched(&B);
  }
}

// TODO: Refactor this nasty function
void StageOneAnalyzer::analyzeSwitchInst(llvm::Instruction *I,
                                         llvm::BasicBlock &B) {
  llvm::SwitchInst *SwI = llvm::cast<llvm::SwitchInst>(I);
  STFREE_LOG(SwI, "[SWITCH] inst found");
  int found_error_case = 0;

  if (auto ICI = llvm::dyn_cast<llvm::ICmpInst>(SwI->getCondition())) {
    if (this->isCallInstReturnValue(ICI->getOperand(0))) {
      for (auto case_element : SwI->cases()) {
        STFREE_LOG_ON(ICI, "Switch inst looking at condition");
        if (case_element.getCaseValue()->getSExtValue() < 0) {
          found_error_case++;
          this->analyzeErrorCode(SwI, ICI, case_element.getCaseSuccessor(), B);
        }
      }

      if (!found_error_case)
        this->analyzeErrorCode(SwI, ICI, SwI->getDefaultDest(), B);
    }
  } else if (llvm::CallInst *CI =
                 llvm::dyn_cast<llvm::CallInst>(SwI->getCondition())) {
    if (getCalledFunction(CI)) {
      STFREE_LOG(I, "[SWITCH] inst looking at is error call inst");

      BasicBlockWorkList allocated_on_err;
      for (auto ele : this->getErrorAllocInCalledFunction(CI, 0)) {
        allocated_on_err.add(ele);
      }
      STFREE_LOG(CI, "Allocated Err: " +
                         std::to_string(allocated_on_err.getList().size()));

      BasicBlockWorkList allocated_on_success;
      for (auto ele : this->getSuccessAllocInCalledFunction(CI)) {
        allocated_on_success.add(ele);
      }
      STFREE_LOG(CI, "Allocated Success: " +
                         std::to_string(allocated_on_success.getList().size()));

      // 2. Success diff allocated_on_err is pure non allocated in err
      BasicBlockList diff_list = BasicBlockListOperation::diffList(
          allocated_on_success.getList(), allocated_on_err.getList());

      STFREE_LOG(CI,
                 "[IS_ERROR] Diff list: " + std::to_string(diff_list.size()));

      for (auto case_element : SwI->cases()) {
        STFREE_LOG_ON(SwI, "Switch inst looking at condition");
        if (case_element.getCaseValue()->getSExtValue() < 0) {
          found_error_case++;
          for (auto ele : diff_list) {
            STFREE_LOG(SwI, "Adding Null value");
            this->getFunctionInformation()
                ->getBasicBlockInformation(&B)
                ->addRemoveAlloc(case_element.getCaseSuccessor(),
                                 const_cast<UniqueKey *>(ele));
          }
        }
      }
      if (!found_error_case)
        for (auto ele : diff_list) {
          STFREE_LOG(SwI, "Adding Null value");
          this->getFunctionInformation()
              ->getBasicBlockInformation(&B)
              ->addRemoveAlloc(SwI->getDefaultDest(),
                               const_cast<UniqueKey *>(ele));
        }
    }
  }
}

void StageOneAnalyzer::analyzeGetElementPtrInst(llvm::Instruction *I,
                                                llvm::BasicBlock &B) {
  llvm::GetElementPtrInst *GEleI = llvm::cast<llvm::GetElementPtrInst>(I);
  // struct collectedInfo info;
  // ParentList plist;

  // Check and see if the parent is local variable or not
  // STFREE_LOG(GEleI, "found GetElementPtrInst");
  // this->collectStructMemberFreeInfo(GEleI, info, plist);

  // Look at the parent and see if that is allocated or not.
  // llvm::GetElementPtrInst *root_gele = this->getRootGEle(GEleI);
  // if (info.indexes.size() > 0 &&
  //     !this->getFunctionInformation()
  //          ->getBasicBlockInformation(&B)
  //          ->getWorkList(ALLOCATED)
  //          .typeExists(root_gele->getPointerOperandType()) &&
  //     get_type(root_gele->getPointerOperandType())->isStructTy() &&
  //     !getFunctionInformation()->isArgValue(info.freeValue)) {
  //   STFREE_LOG(GEleI, "Not allocated nor is an argument val");
  //   getFunctionInformation()->addLocalVar(
  //       &B, get_type(root_gele->getPointerOperandType()), info.freeValue,
  //       GEleI);
  // }
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
        STFREE_LOG(freedStruct->getInst(),
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

    getStructManager()->addCandidateValue(
        &(getFunctionInformation()->getFunction()), strTy, freedStruct);
  }
}
}  // namespace ST_free
