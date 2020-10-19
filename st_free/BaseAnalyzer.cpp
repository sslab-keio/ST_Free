#include "include/BaseAnalyzer.hpp"

#define isEntryPoint(F, B) &(F.getEntryBlock()) == &B ? true : false
#define UpdateIfNull(tgt, cand) (tgt) = ((tgt) == NULL ? (cand) : (tgt))

namespace ST_free {
void BaseAnalyzer::analyzeAdditionalUnknowns(llvm::Function &F) {
  for (llvm::BasicBlock &B : F) {
    for (llvm::Instruction &I : B) {
      if (llvm::StoreInst *SI = llvm::dyn_cast<llvm::StoreInst>(&I))
        this->checkAndChangeActualAuthority(SI);
    }
  }
  return;
}

void BaseAnalyzer::analyze(llvm::Function &F) {
  setFunctionInformation(identifier.getElement(&F));

  if (!getFunctionInformation()->isUnanalyzed()) return;

  getFunctionInformation()->setLoopInformation(getLoopManager()->get(&F));

  int iterate_counter = 0;
  do {
    getFunctionInformation()->setInProgress();
    for (llvm::BasicBlock &B : F) {
      STFREE_LOG(B.getFirstNonPHI(), B.getName());

      getFunctionInformation()->BBCollectInfo(B, isEntryPoint(F, B));
      if (getLoopManager()->IsInLoop(&F, &B))
        getFunctionInformation()->setBasicBlockLoop(
            B, getLoopManager()->getLoop(&F, &B));
      STFREE_LOG_ON(B.getFirstNonPHI(),
                    "Free: " + std::to_string(getFunctionInformation()
                                                  ->getBasicBlockManager()
                                                  ->getBasicBlockFreeList(&B)
                                                  .size()));
      STFREE_LOG_ON(B.getFirstNonPHI(),
                    "Alloc: " + std::to_string(getFunctionInformation()
                                                   ->getBasicBlockManager()
                                                   ->getBasicBlockAllocList(&B)
                                                   .size()));

      this->analyzeInstructions(B);
      STFREE_LOG_ON(B.getFirstNonPHI(),
                    "After Analysis Free: " +
                        std::to_string(getFunctionInformation()
                                           ->getBasicBlockManager()
                                           ->getBasicBlockFreeList(&B)
                                           .size()));
      getFunctionInformation()->updateSuccessorBlock(B);
      getFunctionInformation()->getBasicBlockManager()->shrinkFreedFromAlloc(
          &B);
      if (!getFunctionInformation()
               ->getBasicBlockInformation(&B)
               ->isInformationIdenticalToBackup()) {
        STFREE_LOG(B.getFirstNonPHI(), "Information not Identical");
        getFunctionInformation()->getBasicBlockInformation(&B)->clearBackup();
        getFunctionInformation()->setDirty();
      }
    }
  } while (iterate_counter++ < WORKLIST_MAX_INTERATION &&
           getFunctionInformation()->isDirty());

  this->checkAvailability();
  getFunctionInformation()->setAnalyzed();

  return;
}

void BaseAnalyzer::analyzeInstructions(llvm::BasicBlock &B) {
  for (llvm::Instruction &I : B) {
    if (InstAnalysisMap.find(I.getOpcode()) != InstAnalysisMap.end())
      (this->*InstAnalysisMap[I.getOpcode()])(&I, B);
  }
}

void BaseAnalyzer::analyzeAllocaInst(llvm::Instruction *AI,
                                     llvm::BasicBlock &B) {}

void BaseAnalyzer::analyzeICmpInst(llvm::Instruction *I, llvm::BasicBlock &B) {}

void BaseAnalyzer::analyzeStoreInst(llvm::Instruction *I, llvm::BasicBlock &B) {
  llvm::StoreInst *SI = llvm::cast<llvm::StoreInst>(I);
  if (this->isStoreToStructMember(SI)) {
    STFREE_LOG(SI, "is Store to struct");
    llvm::GetElementPtrInst *GEle = getStoredStruct(SI);
    stManage->addStore(
        llvm::cast<llvm::StructType>(GEle->getSourceElementType()),
        getValueIndices(GEle).back());

    if (llvm::isa<llvm::GlobalValue>(SI->getValueOperand())) {
      STFREE_LOG(SI, "GolbalVariable Store");
      stManage->addGlobalVarStore(
          llvm::cast<llvm::StructType>(GEle->getSourceElementType()),
          getValueIndices(GEle).back());
    }

    if (llvm::isa<llvm::AllocaInst>(SI->getValueOperand())) {
      getFunctionInformation()->setAlias(GEle, SI->getValueOperand());
    }
  }

  if (this->isStoreFromStructMember(SI)) {
    STFREE_LOG(SI, "is Store from struct");
    llvm::GetElementPtrInst *GEle = getStoredStructEle(SI);
    if (llvm::isa<llvm::AllocaInst>(SI->getPointerOperand())) {
      getFunctionInformation()->setAlias(GEle, SI->getPointerOperand());
    }
  }
}

void BaseAnalyzer::analyzeCallInst(llvm::Instruction *I, llvm::BasicBlock &B) {
  llvm::CallInst *CI = llvm::cast<llvm::CallInst>(I);

  if (llvm::Function *called_function = getCalledFunction(CI)) {
    if (isAllocFunction(called_function)) {
      llvm::Value *val = getAllocatedValue(CI);
      if (val != NULL)
        if (llvm::StructType *strTy =
                llvm::dyn_cast<llvm::StructType>(get_type(val->getType()))) {
          stManage->addAlloc(strTy);
        }
      // this->addAlloc(CI, &B);
    } else if (isFreeFunction(called_function)) {
      for (auto arguments = CI->arg_begin(); arguments != CI->arg_end();
           arguments++) {
        this->addFree(llvm::cast<llvm::Value>(arguments), CI, &B);
      }
    } else {
      this->analyzeDifferentFunc((llvm::Function &)(*called_function));
      this->copyArgStatus((llvm::Function &)(*called_function), CI, B);
    }
  }
}

void BaseAnalyzer::analyzeBranchInst(llvm::Instruction *I,
                                     llvm::BasicBlock &B) {
  llvm::BranchInst *BI = llvm::cast<llvm::BranchInst>(I);
  if (this->isCorrectlyBranched(BI)) {
    STFREE_LOG(BI, "Correctly Branched");
    getFunctionInformation()->setCorrectlyBranched(&B);
  }
}

void BaseAnalyzer::analyzeSwitchInst(llvm::Instruction *SwI,
                                     llvm::BasicBlock &B) {}

void BaseAnalyzer::analyzeBitCastInst(llvm::Instruction *I,
                                      llvm::BasicBlock &B) {
  llvm::BitCastInst *BCI = llvm::cast<llvm::BitCastInst>(I);

  llvm::Type *tgtTy = get_type(BCI->getDestTy());
  if (get_type(BCI->getSrcTy())->isIntegerTy()) {
    if (tgtTy->isStructTy()) {
      llvm::Value *V = BCI->getOperand(0);
      getFunctionInformation()->addAliasedType(V, tgtTy);
    }
  }

  if (tgtTy->isIntegerTy()) {
    llvm::Type *srcTy = get_type(BCI->getSrcTy());
    if (srcTy->isStructTy()) {
      llvm::Value *V = BCI->getOperand(0);
      getFunctionInformation()->addAliasedType(V, srcTy);
    }
  }
  return;
}

void BaseAnalyzer::analyzeReturnInst(llvm::Instruction *I,
                                     llvm::BasicBlock &B) {
  llvm::ReturnInst *RI = llvm::cast<llvm::ReturnInst>(I);

  getFunctionInformation()->addEndPoint(&B, RI);
  getFunctionInformation()->clearErrorCodeMap();

  llvm::Type *RetTy = getFunctionInformation()->getFunction().getReturnType();
  if (RetTy->isIntegerTy()) {
    if (RI->getNumOperands() <= 0) return;
    llvm::Value *V = RI->getReturnValue();
    this->checkErrorInstruction(V);
  } else if (RetTy->isPointerTy()) {
    // TODO: add support to pointers
    STFREE_LOG(RI, "[RETURN][POINTER]: No Error Code Analysis");
    llvm::Value *V = RI->getReturnValue();
    this->checkErrorInstruction(V);
  } else {
    STFREE_LOG(RI, "[RETURN]: No Error Code Analysis");
    getFunctionInformation()->addSuccessBlockInformation(&B);
  }
  // #ifdef LOCAL_VARIABLE_ANALYSIS
  //   getFunctionInformation()->freeLocalVarOnReturnBlock(&B);
  // #endif
  return;
}

void BaseAnalyzer::analyzeGetElementPtrInst(llvm::Instruction *I,
                                            llvm::BasicBlock &B) {
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

  for (FreedStruct *freedStruct : fsl) {
    llvm::StructType *strTy =
        llvm::cast<llvm::StructType>(freedStruct->getType());
    int cPointers = strTy->getNumElements();
    std::vector<bool> alreadyFreed = freedStruct->getFreedMember();
    for (int ind = 0; ind < strTy->getNumElements(); ind++) {
      llvm::Type *t = strTy->getElementType(ind);
      if (!t->isPointerTy() || isFuncPointer(t) || alreadyFreed[ind]) continue;

      ValueInformation *vinfo = getFunctionInformation()->getValueInfo(
          freedStruct->getValue(), t, ind);
      if (vinfo != NULL) {
        bool isFreed = false;
        if (getFunctionInformation()->isFreedInBasicBlock(
                freedStruct->getFreedBlock(), vinfo->getValue(), t, ind) ||
            getFunctionInformation()->isCorrectlyBranchedFreeValue(
                freedStruct->getFreedBlock(), vinfo->getValue(), t, ind)) {
          isFreed = true;
          // }
          // else if (!vinfo->noRefCount()) {
          // bool storedValueFreed = false;
          // for(Value * val : vinfo->getAliasList()){
          //     if(getFunctionInformation()->isFreedInBasicBlock(freedStruct->getFreedBlock(),
          //     val, val->getType(), -1)
          //         ||
          //         getFunctionInformation()->isCorrectlyBranchedFreeValue(freedStruct->getFreedBlock(),
          //         val, val->getType(), -1)){ storedValueFreed = true; break;
          //     }
          // }
          // if(storedValueFreed){
          //     cPointers--;
          // }
        }

        if (isFreed) {
          // getFunctionInformation()->setStructMemberFreed(freedStruct,
          //                                                vinfo->getMemberNum());
          if (getFunctionInformation()->isArgValue(vinfo->getValue())) {
            getFunctionInformation()->setStructMemberArgFreed(vinfo->getValue(),
                                                              ParentList());
          }
        }
      }
    }

    if (!getFunctionInformation()->isArgValue(freedStruct->getValue())) {
      stManage->addCandidateValue(&(getFunctionInformation()->getFunction()),
                                  strTy, freedStruct);
    }
  }
  return;
}

bool BaseAnalyzer::isCorrectlyBranched(llvm::BranchInst *BI) {
  if (BI->isConditional() && BI->getCondition() != NULL) {
    if (auto *CmpI = llvm::dyn_cast<llvm::CmpInst>(BI->getCondition())) {
      if (auto *LI = llvm::dyn_cast<llvm::LoadInst>(CmpI->getOperand(0)))
        if (std::string(LI->getPointerOperand()->getName()).find("ref") !=
            std::string::npos)
          return true;
      if (llvm::isa<llvm::ConstantPointerNull>(CmpI->getOperand(1))) {
        return true;
      }
    }
  }
  return false;
}

void BaseAnalyzer::addLocalVariable(llvm::BasicBlock *B, llvm::Type *T,
                                    llvm::Value *V, llvm::Instruction *I,
                                    ParentList P) {
  ValueInformation *vinfo = getFunctionInformation()->addVariable(V);
  getFunctionInformation()->addBasicBlockLiveVariable(B, V);
  if (llvm::StructType *strTy = llvm::dyn_cast<llvm::StructType>(T)) {
    getFunctionInformation()->addLocalVar(B, strTy, V, I, P, vinfo);

    // P.push_back(T);
    for (llvm::Type *ele : strTy->elements()) {
      if (ele->isStructTy()) this->addLocalVariable(B, ele, V, I, P);
    }
  }
  return;
}

void BaseAnalyzer::addPointerLocalVariable(llvm::BasicBlock *B, llvm::Type *T,
                                           llvm::Value *V, llvm::Instruction *I,
                                           ParentList P) {
  ValueInformation *vinfo = getFunctionInformation()->addVariable(V);
  if (llvm::StructType *strTy = llvm::dyn_cast<llvm::StructType>(get_type(T))) {
    getFunctionInformation()->addLocalVar(B, strTy, V, I, P, vinfo);

    // P.push_back(T);
    for (llvm::Type *ele : strTy->elements()) {
      if (ele->isStructTy()) this->addLocalVariable(B, ele, V, I, P);
    }
  }
  return;
}

void BaseAnalyzer::analyzeDifferentFunc(llvm::Function &F) {
  /*** Push current FunctionInformation ***/
  functionStack.push(&getFunctionInformation()->getFunction());

  /*** Analyze new Function ***/
  this->analyze(F);

  /*** Recover FunctionInformation ***/
  llvm::Function *tempFunc = functionStack.top();
  setFunctionInformation(identifier.getElement(tempFunc));
  functionStack.pop();
  return;
}

void BaseAnalyzer::addFree(llvm::Value *V, llvm::CallInst *CI,
                           llvm::BasicBlock *B, bool isAlias,
                           ParentList additionalParents) {
  struct collectedInfo info;
  if (llvm::Instruction *val = llvm::dyn_cast<llvm::Instruction>(V)) {
    if (isStructEleFree(val) || additionalParents.size() > 0) {
      STFREE_LOG_ON(CI, "Struct Element Free");
      this->collectStructMemberFreeInfo(val, info, additionalParents);
    } else if (isOptimizedStructEleFree(val)) {
      STFREE_LOG_ON(CI, "Optimized Struct Element Free");
    }

    if (isStructFree(val)) {
      STFREE_LOG_ON(CI, "Struct Free");
      this->collectStructFreeInfo(val, info);
    } else if (isOptimizedStructFree(val)) {
      STFREE_LOG_ON(CI, "Optimized Struct Free");
      this->collectOptimizedStructFreeInfo(val, info);
    }

    if (!info.isStructRelated) this->collectSimpleFreeInfo(val, info);
  } else {
    // When falling into this branch, it is either a value is optimized arg or
    // it is something else (not really sure). We need to decode this by
    // ourselves This is a temporary implementation.
    // TODO: fix this to more stable implementation.
    STFREE_LOG(CI, "Non Instruction free value found");

    // Get Top-level Value/Type
    llvm::Type *Ty = V->getType();
    for (auto user : V->users()) {
      if (auto BCI = llvm::dyn_cast<llvm::BitCastInst>(user)) {
        Ty = BCI->getDestTy();
      }
    }

    // If additionalParents exists, this means that it is a struct member.
    // Decode everything back.
    if (additionalParents.size() > 0) {
      STFREE_LOG(CI, "reading from additional parents");
      info.indexes = additionalParents;
      info.index = additionalParents.back().second;

      if (auto StTy = llvm::dyn_cast<llvm::StructType>(
              get_type(info.indexes.back().first))) {
        if (0 <= info.index && info.index < StTy->getNumElements())
          UpdateIfNull(info.memType, StTy->getElementType(info.index));
      }
    }

    UpdateIfNull(info.memType, Ty);
    UpdateIfNull(info.freeValue, V);
  }

  if (info.freeValue && !getFunctionInformation()->isFreedInBasicBlock(
                            B, info.freeValue, info.memType, info.index)) {
    STFREE_LOG(CI, "Adding Free Value");
    ValueInformation *valInfo = getFunctionInformation()->addFreeValue(
        B, NULL, info.memType, info.index, info.indexes);

    if (getFunctionInformation()->isArgValue(info.freeValue)) {
      STFREE_LOG(CI, "Add Free Arg");
      valInfo->setArgNumber(
          getFunctionInformation()->getArgIndex(info.freeValue));
      // if (!info.parentType)
      //   getFunctionInformation()->setArgFree(info.freeValue);
      // else if (info.parentType && info.index >= 0) {
      //   STFREE_LOG(CI, "parentType add free arg", true);
      //   getFunctionInformation()->setStructMemberArgFreed(info.freeValue,
      //                                                     info.indexes);
      // }
    }
    if (!isAlias && info.memType && get_type(info.memType)->isStructTy() &&
        this->isAuthorityChained(info.indexes)) {
      STFREE_LOG(CI, "Add Freed Struct");
      getFunctionInformation()->addFreedStruct(
          B, get_type(info.memType), info.freeValue, CI, info.parentType,
          valInfo, info.index != ROOT_INDEX);

      /*** Look for any statically allcated struct type,
       * and add them to freed struct as well ***/
#if defined(OPTION_NESTED)
      this->addNestedFree(V, CI, B, info, additionalParents);
#endif
    }

    if (!isAlias && getFunctionInformation()->aliasExists(info.freeValue)) {
      STFREE_LOG(CI, "Jumping to Alias");
      llvm::Value *aliasVal =
          getFunctionInformation()->getAlias(info.freeValue);
      if (V != aliasVal) this->addFree(aliasVal, CI, B, true);
    }
  }
}

void BaseAnalyzer::addAlloc(llvm::CallInst *CI, llvm::BasicBlock *B) {
  llvm::Type *Ty = CI->getType();
  STFREE_LOG(CI, "Add alloc");
  for (llvm::User *usr : CI->users()) {
    if (auto CastI = llvm::dyn_cast<llvm::CastInst>(usr)) {
      Ty = CastI->getDestTy();
    } else if (auto SI = llvm::dyn_cast<llvm::StoreInst>(usr)) {
      STFREE_LOG(CI, "struct store");
      if (!Ty->isStructTy()) {
        if (auto BCI =
                llvm::dyn_cast<llvm::BitCastInst>(SI->getPointerOperand())) {
          STFREE_LOG(CI, "bit cast found");
          Ty = get_type(BCI->getSrcTy());
        }
      }
    }
  }

  if (get_type(Ty)->isStructTy()) {
    STFREE_LOG(CI, "Stored in Struct");
    getFunctionInformation()->addAliasedType(CI, Ty);
    getStructManager()->addAlloc(llvm::cast<llvm::StructType>(get_type(Ty)));
  }

  const UniqueKey *UK =
      getFunctionInformation()->addAllocValue(B, NULL, Ty, ROOT_INDEX);

  if (!isAllocStoredInSameBasicBlock(CI, B)) {
    STFREE_LOG(CI, "Not Stored in the same block");
    getFunctionInformation()->addPendingAliasedAlloc(UK);
  }

  // The following addAlloc is added for debugging purposes
  getFunctionInformation()->addAllocValue(B, CI, Ty, ROOT_INDEX);
  return;
}

bool BaseAnalyzer::isReturnFunc(llvm::Instruction *I) {
  if (llvm::isa<llvm::ReturnInst>(I)) return true;
  return false;
}

void BaseAnalyzer::copyArgStatus(llvm::Function &Func, llvm::CallInst *CI,
                                 llvm::BasicBlock &B) {
  FunctionInformation *DF = identifier.getElement(&Func);
  int ind = 0;

  for (auto arguments = CI->arg_begin(); arguments != CI->arg_end();
       arguments++, ind++) {
    ArgStatus *args = DF->getArgList()->getArgStatus(ind);
    this->copyArgStatusRecursively(Func, CI, B,
                                   llvm::cast<llvm::Value>(arguments), args,
                                   ind, NULL, ParentList(), true);
  }
  return;
}

void BaseAnalyzer::copyArgStatusRecursively(
    llvm::Function &Func, llvm::CallInst *CI, llvm::BasicBlock &B,
    llvm::Value *arg, ArgStatus *ArgStat, int ind, llvm::Type *ParentType,
    ParentList plist, bool isFirst) {
  if (ArgStat && ArgStat->isStruct()) {
    STFREE_LOG(CI, "Args is Struct");
    if (!isFirst)
      plist.push_back(std::pair<llvm::Type *, int>(ParentType, ind));

    if (ArgStat->isFreed()) {
      STFREE_LOG(CI, "Copy Args Stat");
      this->addFree(arg, CI, &B, false, plist);
      llvm::Type *T = get_type(ArgStat->getType());
      if (llvm::isa<llvm::StructType>(T))
        getFunctionInformation()->copyStructMemberFreed(
            T, ArgStat->getFreedList());
    }

    for (int index = 0; index < ArgStat->size(); index++) {
      this->copyArgStatusRecursively(Func, CI, B, arg,
                                     ArgStat->getStatus(index), index,
                                     get_type(ArgStat->getType()), plist);
    }
  }
}

void BaseAnalyzer::copyAllocatedStatus(llvm::Function &Func, llvm::CallInst *CI,
                                       llvm::BasicBlock &B) {
  FunctionInformation *DF = identifier.getElement(&Func);
  STFREE_LOG(
      CI, "Copied alloc: " + std::to_string(DF->getAllocatedInReturn().size()));
  for (auto ele : DF->getAllocatedInReturn()) {
    getFunctionInformation()->addAllocValue(&B, const_cast<UniqueKey *>(ele));
  }
  return;
}

void BaseAnalyzer::copyFreeStatus(llvm::Function &Func, llvm::CallInst *CI,
                                  llvm::BasicBlock &B) {
  FunctionInformation *DF = identifier.getElement(&Func);
  STFREE_LOG_ON(CI, "Copy Free Status");
  for (auto ele : DF->getFreedInSuccess()) {
    STFREE_LOG_ON(CI, "Copying Free Status " +
                          std::to_string(DF->getFreedInSuccess().size()));
    if (ValueInformation *vinfo = DF->getValueInfo(ele)) {
      if (vinfo->isArgValue()) {
        STFREE_LOG(CI, "Copying value");
        if (vinfo->getArgNumber() < CI->getNumArgOperands()) {
          STFREE_LOG_ON(CI, "Copying value");
          addFree(CI->getArgOperand(vinfo->getArgNumber()), CI, &B, false,
                  vinfo->getParents());
        }
      } else {
        STFREE_LOG(CI, "Falling into this pit");
        getFunctionInformation()->addFreeValue(&B,
                                               const_cast<UniqueKey *>(ele));
      }
    }
  }
  return;
}

void BaseAnalyzer::evaluatePendingStoredValue(llvm::Function &Func,
                                              llvm::CallInst *CI,
                                              llvm::BasicBlock &B) {
  FunctionInformation *DF = identifier.getElement(&Func);
  for (auto ele : DF->getPendingStoreInReturn()) {
    const UniqueKey *UK = const_cast<UniqueKey *>(ele);
    if (this->getFunctionInformation()
            ->getBasicBlockInformation(&B)
            ->getWorkList(ALLOCATED)
            .typeExists(UK->getType())) {
      getFunctionInformation()->addAllocValue(&B, const_cast<UniqueKey *>(ele));
    } else {
      // Determine whether it needs further lazy evaluation, or the lifetime
      // can end here
    }
  }
}

llvm::CallInst *BaseAnalyzer::getStoreFromCall(llvm::StoreInst *SI) {
  llvm::Value *val = SI->getValueOperand();
  if (auto BCI = llvm::dyn_cast<llvm::CastInst>(val)) {
    val = BCI->getOperand(0);
  }
  if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(val)) return CI;
  return NULL;
}

bool BaseAnalyzer::isStoreToStructMember(llvm::StoreInst *SI) {
  llvm::Value *v = SI->getPointerOperand();
  if (auto LI = llvm::dyn_cast<llvm::LoadInst>(v)) v = LI->getPointerOperand();
  if (auto BCI = llvm::dyn_cast<llvm::CastInst>(v)) v = BCI->getOperand(0);
  if (llvm::GetElementPtrInst *gEle =
          llvm::dyn_cast<llvm::GetElementPtrInst>(v)) {
    return true;
  }
  return false;
}

bool BaseAnalyzer::isStoreFromStructMember(llvm::StoreInst *SI) {
  if (getStoredStructEle(SI)) return true;
  return false;
}

bool BaseAnalyzer::isStoreToStruct(llvm::StoreInst *SI) {
  llvm::Type *Ty = SI->getPointerOperandType();
  if (auto CI = llvm::dyn_cast<llvm::CastInst>(SI->getPointerOperand())) {
    Ty = CI->getSrcTy();
  }

  if (Ty->isPointerTy()) {
    if (get_type(Ty)->isStructTy()) return true;
  }
  return false;
}

llvm::StructType *BaseAnalyzer::getStoreeStruct(llvm::StoreInst *SI) {
  llvm::Type *Ty = SI->getPointerOperandType();
  if (auto CI = llvm::dyn_cast<llvm::CastInst>(SI->getPointerOperand())) {
    Ty = CI->getSrcTy();
  }

  if (Ty->isPointerTy()) {
    if (auto StTy = llvm::dyn_cast<llvm::StructType>(get_type(Ty))) return StTy;
  }
  return NULL;
}

llvm::StructType *BaseAnalyzer::getStorerStruct(llvm::StoreInst *SI) {
  llvm::Type *Ty = SI->getValueOperand()->getType();
  if (auto CI = llvm::dyn_cast<llvm::CastInst>(SI->getValueOperand())) {
    Ty = CI->getSrcTy();
  }

  if (Ty->isPointerTy()) {
    if (auto StTy = llvm::dyn_cast<llvm::StructType>(get_type(Ty))) return StTy;
  }
  return NULL;
}

bool BaseAnalyzer::isStoreFromStruct(llvm::StoreInst *SI) {
  if (get_type(SI->getValueOperand()->getType())->isStructTy()) return true;
  return false;
}

void BaseAnalyzer::checkAndChangeActualAuthority(llvm::StoreInst *SI) {
  std::vector<llvm::CastInst *> CastInsts;
  if (this->isStoreToStructMember(SI)) {
    llvm::GetElementPtrInst *GEle = getStoredStruct(SI);
    // ParentList indexes;
    // this->getStructParents(GEle, indexes);
    if (GEle && llvm::isa<llvm::StructType>(GEle->getSourceElementType())) {
      STFREE_LOG(SI, "Found StoreInst to struct member");

      if (auto PN = llvm::dyn_cast<llvm::PHINode>(SI->getValueOperand())) {
        STFREE_LOG(SI, "is PhiInst");
        for (int i = 0; i < PN->getNumIncomingValues(); i++) {
          if (auto CI = llvm::dyn_cast<llvm::CastInst>(PN->getIncomingValue(i)))
            CastInsts.push_back(CI);
        }
      } else if (auto CI =
                     llvm::dyn_cast<llvm::CastInst>(SI->getValueOperand())) {
        CastInsts.push_back(CI);
      }

      for (auto CI : CastInsts) this->changeAuthority(SI, CI, GEle);
    }
  }

  // if (this->isStoreFromStructMember(SI)) {
  //   llvm::GetElementPtrInst *GEle = getStoredStructEle(SI);
  //   ParentList indexes;
  // }
}

void BaseAnalyzer::changeAuthority(llvm::StoreInst *SI, llvm::CastInst *CI,
                                   llvm::GetElementPtrInst *GEle) {
  ParentList indexes;
  STFREE_LOG(SI, "is Casted Store");
  this->getStructParents(GEle, indexes);

  if (indexes.size() > 0 &&
      this->isAuthorityChained(std::vector<std::pair<llvm::Type *, int>>(
          indexes.end() - 1, indexes.end())) &&
      !this->isAllocCast(CI) && !this->isCastToVoid(CI)) {
    if (llvm::StructType *StTy =
            llvm::dyn_cast<llvm::StructType>(indexes.back().first)) {
      STFREE_LOG(SI, "Change back to Unknown");
      getStructManager()->get(StTy)->setMemberStatUnknown(
          indexes.back().second);
    }
  }
  return;
}

bool BaseAnalyzer::isDirectStoreFromAlloc(llvm::StoreInst *SI) {
  if (auto CI = llvm::dyn_cast<llvm::CallInst>(SI->getValueOperand())) {
    if (isAllocFunction(getCalledFunction(CI))) return true;
  }
  return false;
}

std::vector<std::string> BaseAnalyzer::decodeDirectoryName(std::string fname) {
  std::vector<std::string> dirs;
  size_t pos;

  while ((pos = fname.find("/")) != std::string::npos) {
    std::string token = fname.substr(0, pos);
    dirs.push_back(token);
    fname.erase(0, pos + 1);
  }
  dirs.push_back(fname);

  return dirs;
}

void BaseAnalyzer::getStructParents(
    llvm::Instruction *I, std::vector<std::pair<llvm::Type *, int>> &typeList) {
  if (llvm::LoadInst *LI = llvm::dyn_cast<llvm::LoadInst>(I)) {
    if (llvm::Instruction *Inst =
            llvm::dyn_cast<llvm::Instruction>(LI->getPointerOperand()))
      this->getStructParents(Inst, typeList);
  } else if (llvm::GetElementPtrInst *GI =
                 llvm::dyn_cast<llvm::GetElementPtrInst>(I)) {
    if (llvm::Instruction *Inst =
            llvm::dyn_cast<llvm::Instruction>(GI->getPointerOperand()))
      this->getStructParents(Inst, typeList);
    std::vector<std::pair<llvm::Type *, long>> decoded_vals =
        this->decodeGEPInst(GI);
    for (auto dec : decoded_vals) {
      if (dec.first != NULL && dec.second != ROOT_INDEX)
        typeList.push_back(std::pair<llvm::Type *, int>(dec.first, dec.second));
    }
  }
  return;
}

long BaseAnalyzer::getMemberIndiceFromByte(llvm::StructType *STy,
                                           uint64_t byte) {
  const llvm::StructLayout *sl = this->getStructLayout(STy);
  if (sl != NULL) return sl->getElementContainingOffset(byte);
  return ROOT_INDEX;
}

std::vector<long> BaseAnalyzer::getValueIndices(llvm::GetElementPtrInst *inst) {
  long indice = ROOT_INDEX;
  std::vector<long> indices;

  llvm::Type *Ty = inst->getSourceElementType();
  auto idx_itr = inst->idx_begin();
  if (!Ty->isIntegerTy()) idx_itr++;

  // for(auto idx_itr = inst->idx_begin() + 1; idx_itr != inst->idx_end();
  // idx_itr++) {
  for (; idx_itr != inst->idx_end(); idx_itr++) {
    if (llvm::ConstantInt *cint =
            llvm::dyn_cast<llvm::ConstantInt>(idx_itr->get()))
      indice = cint->getSExtValue();
    else
      indice = ROOT_INDEX;
    indices.push_back(indice);
  }

  return indices;
}

llvm::GetElementPtrInst *BaseAnalyzer::getRootGEle(
    llvm::GetElementPtrInst *GEle) {
  llvm::GetElementPtrInst *tgt = GEle;
  while (llvm::isa<llvm::GetElementPtrInst>(tgt->getPointerOperand())) {
    tgt = llvm::cast<llvm::GetElementPtrInst>(tgt->getPointerOperand());
  }
  return tgt;
}

bool BaseAnalyzer::isStructEleAlloc(llvm::Instruction *val) {
  for (llvm::User *usr : val->users()) {
    llvm::User *tmp_usr = usr;
    if (!llvm::isa<llvm::StoreInst>(usr)) {
      for (llvm::User *neo_usr : usr->users()) {
        if (llvm::isa<llvm::StoreInst>(neo_usr)) {
          tmp_usr = neo_usr;
          break;
        }
      }
    }
    if (llvm::StoreInst *str_inst = llvm::dyn_cast<llvm::StoreInst>(tmp_usr)) {
      llvm::Value *tgt_op = str_inst->getOperand(1);
      if (llvm::GetElementPtrInst *inst =
              llvm::dyn_cast<llvm::GetElementPtrInst>(tgt_op)) {
        return true;
      }
    }
  }
  return false;
}

llvm::Value *BaseAnalyzer::getAllocatedValue(llvm::Instruction *val) {
  for (llvm::User *usr : val->users()) {
    llvm::User *tmp_usr = usr;
    if (!llvm::isa<llvm::StoreInst>(usr)) {
      for (llvm::User *neo_usr : usr->users()) {
        if (llvm::isa<llvm::StoreInst>(neo_usr)) {
          tmp_usr = neo_usr;
          break;
        }
      }
    }
    if (llvm::StoreInst *str_inst = llvm::dyn_cast<llvm::StoreInst>(tmp_usr)) {
      llvm::Value *tgt_op = str_inst->getOperand(1);
      return tgt_op;
    }
  }
  return NULL;
}

llvm::GetElementPtrInst *BaseAnalyzer::getAllocStructEleInfo(
    llvm::Instruction *val) {
  for (llvm::User *usr : val->users()) {
    llvm::User *tmp_usr = usr;
    if (!llvm::isa<llvm::StoreInst>(usr)) {
      for (llvm::User *neo_usr : usr->users()) {
        if (llvm::isa<llvm::StoreInst>(neo_usr)) {
          tmp_usr = neo_usr;
          break;
        }
      }
    }
    if (llvm::StoreInst *str_inst = llvm::dyn_cast<llvm::StoreInst>(tmp_usr)) {
      llvm::Value *tgt_op = str_inst->getOperand(1);
      if (llvm::GetElementPtrInst *inst =
              llvm::dyn_cast<llvm::GetElementPtrInst>(tgt_op)) {
        return inst;
      }
    }
  }
  return NULL;
}

bool BaseAnalyzer::isStructEleFree(llvm::Instruction *val) {
  if (llvm::isa<llvm::GetElementPtrInst>(val)) return true;

  llvm::LoadInst *l_inst = find_load(val);
  if (l_inst && l_inst->getOperandList()) {
    llvm::Value *V = l_inst->getPointerOperand();
    if (auto bit_cast_inst = llvm::dyn_cast<llvm::BitCastInst>(V)) {
      STFREE_LOG(val, "found BitCast");
      V = bit_cast_inst->getOperand(0);
    }
    // if (auto itoptr_inst = llvm::dyn_cast<llvm::IntToPtrInst>(V)) {
    //   STFREE_LOG(val, "found IntToPtrInst");
    //   V = itoptr_inst->getOperand(0);
    // }
    if (auto GEle = llvm::dyn_cast<llvm::GetElementPtrInst>(V)) {
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

llvm::GetElementPtrInst *BaseAnalyzer::getFreeStructEleInfo(
    llvm::Instruction *val) {
  if (auto GEle = llvm::dyn_cast<llvm::GetElementPtrInst>(val)) return GEle;

  llvm::LoadInst *l_inst = find_load(val);
  if (l_inst != NULL && l_inst->getOperandList() != NULL) {
    llvm::Value *V = l_inst->getPointerOperand();
    if (auto bit_cast_inst = llvm::dyn_cast<llvm::BitCastInst>(V)) {
      STFREE_LOG(val, "found BitCast");
      V = bit_cast_inst->getOperand(0);
    }
    if (auto GEle = llvm::dyn_cast<llvm::GetElementPtrInst>(V)) {
      return GEle;
    }
  }
  return NULL;
}

bool BaseAnalyzer::isStructFree(llvm::Instruction *val) {
  if (getStructFreedValue(val) != NULL) return true;
  return false;
}

bool BaseAnalyzer::isOptimizedStructFree(llvm::Instruction *I) {
  return getFunctionInformation()->aliasedTypeExists(I);
}

bool BaseAnalyzer::isOptimizedStructEleFree(llvm::Instruction *I) {
  if (auto cast_inst = llvm::dyn_cast<llvm::CastInst>(I)) {
    generateWarning(I, "Found cast inst here", true);
    if (auto StTy =
            llvm::dyn_cast<llvm::StructType>(get_type(cast_inst->getSrcTy()))) {
      generateWarning(I, "src is struct", true);
      if (StTy->getNumElements() > 0)
        return StTy->getElementType(0) == get_type(cast_inst->getDestTy());
    }
  }
  return false;
}

llvm::Type *BaseAnalyzer::getOptimizedStructFree(llvm::Instruction *I) {
  return getFunctionInformation()->getAliasedType(I);
}

llvm::Type *BaseAnalyzer::getStructType(llvm::Instruction *val) {
  llvm::LoadInst *load_inst = find_load(val);
  if (load_inst && load_inst->getOperandList() != NULL) {
    llvm::Type *tgt_type = get_type(load_inst->getPointerOperandType());
    if (tgt_type && get_type(tgt_type)->isStructTy()) return tgt_type;
  } else if (auto *BCI = llvm::dyn_cast<llvm::BitCastInst>(val)) {
    if (get_type(BCI->getSrcTy())->isStructTy()) return BCI->getSrcTy();
  }
  return NULL;
}

bool BaseAnalyzer::isFuncPointer(llvm::Type *t) {
  llvm::Type *tgt = get_type(t);
  if (tgt->isFunctionTy()) return true;
  return false;
}

llvm::Value *BaseAnalyzer::getStructFreedValue(llvm::Instruction *val,
                                               bool isUserDefCalled) {
  llvm::LoadInst *load_inst = find_load(val);
  if (load_inst && load_inst->getOperandList() != NULL) {
    llvm::Type *tgt_type = get_type(load_inst->getPointerOperandType());
    if (auto BCI =
            llvm::dyn_cast<llvm::BitCastInst>(load_inst->getPointerOperand())) {
      tgt_type = BCI->getSrcTy();
    }
    if (tgt_type)
      if (llvm::isa<llvm::StructType>(get_type(tgt_type))) {
        return getLoadeeValue(load_inst);
      }
  } else if (isUserDefCalled) {
    llvm::Value *V = val;
    if (auto SelI = llvm::dyn_cast<llvm::SelectInst>(val))
      V = SelI->getCondition();
    if (auto *BCI = llvm::dyn_cast<llvm::BitCastInst>(val))
      V = BCI->getOperand(0);

    if (this->getFunctionInformation()->aliasedTypeExists(V))
      if (llvm::isa<llvm::StructType>(
              get_type(this->getFunctionInformation()->getAliasedType(V))))
        return V;
  } else if (auto *BCI = llvm::dyn_cast<llvm::BitCastInst>(val)) {
    if (get_type(BCI->getSrcTy())->isStructTy()) return BCI->getOperand(0);
  } else if (auto ICI = llvm::dyn_cast<llvm::IntToPtrInst>(val)) {
    STFREE_LOG_ON(val, "IntToPtrInst");
    if (auto def_inst = llvm::dyn_cast<llvm::Instruction>(ICI->getOperand(0)))
      return getStructFreedValue(def_inst);
  }
  return NULL;
}

llvm::Value *BaseAnalyzer::getCalledStructFreedValue(llvm::Instruction *val) {
  return this->getStructFreedValue(val, true);
}

llvm::Value *BaseAnalyzer::getFreedValue(llvm::Instruction *val) {
  llvm::LoadInst *load_inst = find_load(val);
  if (load_inst != NULL && load_inst->getOperandList() != NULL)
    return getLoadeeValue(load_inst);
  return NULL;
}

llvm::GetElementPtrInst *BaseAnalyzer::getStoredStructEle(llvm::StoreInst *SI) {
  llvm::Value *val = SI->getValueOperand();
  if (auto LInst = llvm::dyn_cast<llvm::LoadInst>(val))
    val = LInst->getPointerOperand();
  if (auto GEle = llvm::dyn_cast<llvm::GetElementPtrInst>(val)) return GEle;
  return NULL;
}

llvm::GetElementPtrInst *BaseAnalyzer::getStoredStruct(llvm::StoreInst *SI) {
  llvm::Value *v = SI->getPointerOperand();
  if (auto BCI = llvm::dyn_cast<llvm::CastInst>(v)) v = BCI->getOperand(0);
  if (llvm::GetElementPtrInst *GEle =
          llvm::dyn_cast<llvm::GetElementPtrInst>(v))
    return GEle;
  return NULL;
}

std::vector<std::pair<llvm::Type *, long>> BaseAnalyzer::decodeGEPInst(
    llvm::GetElementPtrInst *GEle) {
  llvm::Type *Ty = GEle->getSourceElementType();
  std::vector<long> indice = getValueIndices(GEle);
  std::vector<std::pair<llvm::Type *, long>> decoded;

  for (long ind : indice) {
    long index = ind;
    if (Ty && index != ROOT_INDEX) {
      if (Ty->isIntegerTy()) {
        if (getFunctionInformation()->aliasedTypeExists(
                GEle->getPointerOperand()) &&
            decoded.empty())
          Ty = getFunctionInformation()->getAliasedType(
              GEle->getPointerOperand());
        if (Ty && get_type(Ty)->isStructTy()) {
          index = getMemberIndiceFromByte(
              llvm::cast<llvm::StructType>(get_type(Ty)), index);
        }
      }
      decoded.push_back(std::pair<llvm::Type *, long>(Ty, index));
      if (auto StTy = llvm::dyn_cast<llvm::StructType>(Ty))
        Ty = StTy->getElementType(index);
    }
  }

  return decoded;
}

llvm::Type *BaseAnalyzer::extractResultElementType(
    llvm::GetElementPtrInst *GEle) {
  llvm::Type *Ty = GEle->getResultElementType();

  if (get_type(Ty)->isIntegerTy()) {
    for (llvm::User *usr : GEle->users()) {
      if (auto BCI = llvm::dyn_cast<llvm::BitCastInst>(usr)) {
        Ty = get_type(BCI->getDestTy());
      }
    }
  }
  return Ty;
}

bool BaseAnalyzer::isAuthorityChained(ParentList pt) {
  for (std::pair<llvm::Type *, long> ele : pt) {
    if (auto StTy = llvm::dyn_cast<llvm::StructType>(get_type(ele.first))) {
      if (!stManage->structHoldsAuthority(StTy, ele.second)) return false;
    }
  }
  return true;
}

bool BaseAnalyzer::isAllocCast(llvm::CastInst *cast) {
  if (auto CI = llvm::dyn_cast<llvm::CallInst>(cast->getOperand(0)))
    if (isAllocFunction(getCalledFunction(CI))) return true;
  return false;
}

bool BaseAnalyzer::isCastToVoid(llvm::CastInst *CI) {
  // TODO: need more fine-grained checks
  if (auto BCI = llvm::dyn_cast<llvm::BitCastInst>(CI)) {
    if (get_type(BCI->getDestTy())->isIntegerTy()) {
      return true;
    }
  } else if (llvm::isa<llvm::PtrToIntInst>(CI)) {
    return true;
  }
  return false;
}

void BaseAnalyzer::collectStructMemberFreeInfo(
    llvm::Instruction *I, struct BaseAnalyzer::collectedInfo &info,
    ParentList &additionalParents) {
  llvm::GetElementPtrInst *GEle = getFreeStructEleInfo(I);
  if (GEle != NULL) {
    STFREE_LOG_ON(I, "GetElementPtr Inst found");
    this->getStructParents(GEle, info.indexes);
    llvm::GetElementPtrInst *tmpGEle = GEle;
    if (llvm::isa<llvm::GetElementPtrInst>(GEle->getPointerOperand()))
      tmpGEle = getRootGEle(GEle);
    UpdateIfNull(info.freeValue, getLoadeeValue(tmpGEle->getPointerOperand()));
  }

  for (auto addParent : additionalParents) info.indexes.push_back(addParent);

  if (info.indexes.size() > 0) {
    info.index = info.indexes.back().second;

    if (auto StTy = llvm::dyn_cast<llvm::StructType>(
            get_type(info.indexes.back().first))) {
      if (0 <= info.index && info.index < StTy->getNumElements())
        UpdateIfNull(info.memType, StTy->getElementType(info.index));
      else if (ROOT_INDEX < info.index) {
        // STFREE_LOG(I, "[Index Over?] Exceeded: " +
        // std::to_string(info.index), true);
        // TODO: add solid support to negative indice of GEP (a.k.a.
        // container_of)
      }
    }

    if (get_type(info.indexes.front().first)->isStructTy())
      UpdateIfNull(info.parentType, llvm::cast<llvm::StructType>(
                                        get_type(info.indexes.front().first)));

    UpdateIfNull(info.freeValue, getCalledStructFreedValue(I));
    UpdateIfNull(info.freeValue, I);
    info.isStructRelated = true;
    STFREE_LOG_ON(I, "Struct element free collected");
  }
  return;
}

void BaseAnalyzer::collectSimpleFreeInfo(
    llvm::Instruction *I, struct BaseAnalyzer::collectedInfo &info) {
  UpdateIfNull(info.freeValue, getFreedValue(I));
  if (info.freeValue) UpdateIfNull(info.memType, info.freeValue->getType());
  STFREE_LOG(I, "Value Free");
  return;
}

void BaseAnalyzer::collectStructFreeInfo(
    llvm::Instruction *I, struct BaseAnalyzer::collectedInfo &info) {
  llvm::Value *loaded_value = getStructFreedValue(I);
  if (loaded_value) {
    UpdateIfNull(info.freeValue, loaded_value);
    UpdateIfNull(info.memType, getStructType(I));
  }
  info.isStructRelated = true;
  return;
}

void BaseAnalyzer::collectOptimizedStructFreeInfo(
    llvm::Instruction *I, struct BaseAnalyzer::collectedInfo &info) {
  UpdateIfNull(info.freeValue, I);
  UpdateIfNull(info.memType, getOptimizedStructFree(I));
  info.isStructRelated = true;
  return;
}

void BaseAnalyzer::addNestedFree(llvm::Value *V, llvm::CallInst *CI,
                                 llvm::BasicBlock *B,
                                 struct collectedInfo &info,
                                 ParentList &additionalParents) {
  llvm::StructType *StTy = llvm::cast<llvm::StructType>(get_type(info.memType));
  int memIndex = 0;

  for (auto ele = StTy->element_begin(); ele != StTy->element_end();
       ele++, memIndex++) {
    if ((*ele)->isStructTy() &&
        find_if(info.indexes.begin(), info.indexes.end(),
                [ele](const std::pair<llvm::Type *, int> &index) {
                  return *ele == index.first;
                }) == info.indexes.end()) {
      STFREE_LOG(CI, "Option Nested Called");
      additionalParents.push_back(std::pair<llvm::Type *, int>(StTy, memIndex));
      this->addFree(V, CI, B, false, additionalParents);
      additionalParents.pop_back();
    }
  }
  return;
}

void BaseAnalyzer::addRefcountedFree(llvm::Value *V, llvm::CallInst *CI,
                                     llvm::BasicBlock *B) {
  llvm::Value *decoded_value = V;
  STFREE_LOG(CI, "Add Refcounted Free");
  if (llvm::Instruction *I = llvm::dyn_cast<llvm::Instruction>(V)) {
    if (llvm::GetElementPtrInst *GEle = getFreeStructEleInfo(I))
      decoded_value = GEle->getPointerOperand();
  }
  this->addFree(decoded_value, CI, B);
}

llvm::ICmpInst *BaseAnalyzer::findAllocICmp(llvm::Instruction *I) {
  llvm::ICmpInst *icmp = NULL;

  for (auto usr : I->users()) {
    if (auto ip = llvm::dyn_cast<llvm::ICmpInst>(usr)) {
      icmp = ip;
      break;
    } else if (auto BCI = llvm::dyn_cast<llvm::BitCastInst>(usr)) {
      icmp = findAllocICmp(BCI);
    } else if (auto SI = llvm::dyn_cast<llvm::StoreInst>(usr)) {
      for (auto si_usr : SI->getPointerOperand()->users()) {
        if (auto tmpI = llvm::dyn_cast<llvm::Instruction>(si_usr)) {
          icmp = findAllocICmp(tmpI);
        }
      }
    }
  }
  return icmp;
}

void BaseAnalyzer::analyzeErrorCode(llvm::BranchInst *BI, llvm::ICmpInst *ICI,
                                    llvm::BasicBlock &B) {
  int op = this->getErrorOperand(ICI);
  llvm::CmpInst::Predicate pred = ICI->getPredicate();

  // TODO: update to adjust errocode information
  int errcode = 0;

  if (op >= 0) {
    STFREE_LOG_ON(BI, "Analyzing Error Code");
    if (this->errorCodeExists(ICI, B, errcode)) {
      llvm::BasicBlock *errBlock = BI->getSuccessor(op);
      this->getFunctionInformation()
          ->getBasicBlockInformation(&B)
          ->addSucceedingErrorBlock(errBlock);

      BasicBlockWorkList allocated_on_err =
          this->getErrorValues(ICI, B, errcode);
      STFREE_LOG_ON(BI, "Allocated Error: " +
                            std::to_string(allocated_on_err.getList().size()));

      // 1. Get Allocated on Success
      BasicBlockWorkList allocated_on_success = this->getSuccessValues(ICI, B);
      STFREE_LOG_ON(BI,
                    "Allocated Success: " +
                        std::to_string(allocated_on_success.getList().size()));

      // 2. Success diff allocated_on_err is pure non allocated in err
      BasicBlockList diff_list = BasicBlockListOperation::diffList(
          allocated_on_success.getList(), allocated_on_err.getList());

      diff_list = BasicBlockListOperation::uniteList(
          diff_list, getErrorFreedValues(ICI, B, errcode).getList());

      STFREE_LOG_ON(BI, "Diff list: " + std::to_string(diff_list.size()));

      // 3. add it to the list
      for (auto ele : diff_list) {
        if (ele->getType() != NULL)
          this->getFunctionInformation()
              ->getBasicBlockInformation(&B)
              ->addRemoveAlloc(errBlock, const_cast<UniqueKey *>(ele));
      }
    }
  }
}

void BaseAnalyzer::analyzeErrorCode(llvm::SwitchInst *SwI, llvm::ICmpInst *ICI,
                                    llvm::BasicBlock *tgtBlock,
                                    llvm::BasicBlock &B) {
  int op = this->getErrorOperand(ICI);
  llvm::CmpInst::Predicate pred = ICI->getPredicate();

  // TODO: update to adjust errocode information
  int errcode = 0;
  if (op >= 0) {
    STFREE_LOG_ON(SwI, "Analyzing Error Code");
    if (this->errorCodeExists(ICI, B, errcode)) {
      this->getFunctionInformation()
          ->getBasicBlockInformation(&B)
          ->addSucceedingErrorBlock(tgtBlock);

      BasicBlockWorkList allocated_on_err =
          this->getErrorValues(ICI, B, errcode);
      STFREE_LOG_ON(SwI, "Allocated Err: " +
                             std::to_string(allocated_on_err.getList().size()));

      // 1. Get Allocated on Success
      BasicBlockWorkList allocated_on_success = this->getSuccessValues(ICI, B);
      STFREE_LOG_ON(SwI,
                    "Allocated Success: " +
                        std::to_string(allocated_on_success.getList().size()));

      // 2. Success diff allocated_on_err is pure non allocated in err
      BasicBlockList diff_list = BasicBlockListOperation::diffList(
          allocated_on_success.getList(), allocated_on_err.getList());
      // 3. add it to the list
      for (auto ele : diff_list) {
        this->getFunctionInformation()
            ->getBasicBlockInformation(&B)
            ->addRemoveAlloc(tgtBlock, const_cast<UniqueKey *>(ele));
      }
    }
  }
}

void BaseAnalyzer::analyzeNullCheck(llvm::BranchInst *BI, llvm::ICmpInst *ICI,
                                    llvm::BasicBlock &B) {
  STFREE_LOG(BI, "Analyze NULL Check");
  BasicBlockWorkList BList;

  // Get which basicblock to pass the data to
  int op = this->getErrorOperand(ICI);
  llvm::BasicBlock *errBlock = BI->getSuccessor(op);

  // decode compared type
  llvm::Value *comVal = this->getComparedValue(ICI);
  llvm::Type *Ty = this->getComparedType(comVal, B);

  // check if the value is struct or not
  ParentList plist = this->decodeErrorTypes(ICI->getOperand(0));
  if (plist.size() > 0) {
    if (auto StTy =
            llvm::dyn_cast<llvm::StructType>(get_type(plist.back().first))) {
      if (0 <= plist.back().second &&
          plist.back().second < StTy->getNumElements())
        Ty = StTy->getElementType(plist.back().second);
    }
    BList.add(
        this->getFunctionInformation()->getUniqueKeyManager()->getUniqueKey(
            NULL, Ty, plist.back().second));
  }

  BList.add(this->getFunctionInformation()->getUniqueKeyManager()->getUniqueKey(
      NULL, Ty, ROOT_INDEX));

  for (auto ele : BList.getList()) {
    STFREE_LOG(BI, "Adding Null value");
    this->getFunctionInformation()
        ->getBasicBlockInformation(&B)
        ->addRemoveAlloc(errBlock, const_cast<UniqueKey *>(ele));
  }
}

void BaseAnalyzer::analyzeErrorCheckFunction(llvm::BranchInst *BI,
                                             llvm::CallInst *CI,
                                             llvm::BasicBlock &B) {
  STFREE_LOG_ON(CI, "Calling IS_ERR()");
  BasicBlockWorkList BList;

  llvm::BasicBlock *errBlock = BI->getSuccessor(0);
  this->getFunctionInformation()
      ->getBasicBlockInformation(&B)
      ->addSucceedingErrorBlock(errBlock);

  llvm::Value *tgt_val = CI->getArgOperand(0);
  if (auto BCI = llvm::dyn_cast<llvm::BitCastInst>(tgt_val)) {
    tgt_val = BCI->getOperand(0);
  }

  if (auto error_call_inst = llvm::dyn_cast<llvm::CallInst>(tgt_val)) {
    STFREE_LOG_ON(CI, "[IS_ERR] Value inherited from CallInst");
    if (getCalledFunction(error_call_inst))
      STFREE_LOG_ON(error_call_inst, "Does not have called function!");

    BasicBlockWorkList allocated_on_err;
    for (auto ele : this->getErrorAllocInCalledFunction(error_call_inst, 0)) {
      allocated_on_err.add(ele);
    }
    STFREE_LOG_ON(BI, "Allocated Err: " +
                          std::to_string(allocated_on_err.getList().size()));

    BasicBlockWorkList allocated_on_success;
    for (auto ele : this->getSuccessAllocInCalledFunction(error_call_inst)) {
      allocated_on_success.add(ele);
    }
    STFREE_LOG(BI, "Allocated Success: " +
                       std::to_string(allocated_on_success.getList().size()));

    // 2. Success diff allocated_on_err is pure non allocated in err
    BasicBlockList diff_list = BasicBlockListOperation::diffList(
        allocated_on_success.getList(), allocated_on_err.getList());

    STFREE_LOG_ON(BI,
                  "[IS_ERROR] Diff list: " + std::to_string(diff_list.size()));
    // 3. add it to the list
    for (auto ele : diff_list) {
      BList.add(ele);
    }
  }

  ParentList plist = this->decodeErrorTypes(tgt_val);
  llvm::Type *Ty = this->getComparedType(decodeComparedValue(tgt_val), B);
  if (plist.size() > 0) {
    STFREE_LOG_ON(CI, "Calling IS_ERR(): plist");
    if (auto StTy =
            llvm::dyn_cast<llvm::StructType>(get_type(plist.back().first))) {
      if (0 <= plist.back().second &&
          plist.back().second < StTy->getNumElements())
        Ty = StTy->getElementType(plist.back().second);
    }
    BList.add(
        this->getFunctionInformation()->getUniqueKeyManager()->getUniqueKey(
            NULL, Ty, plist.back().second));
  }
  BList.add(this->getFunctionInformation()->getUniqueKeyManager()->getUniqueKey(
      NULL, Ty, ROOT_INDEX));

  for (auto ele : BList.getList()) {
    STFREE_LOG(BI, "Adding Null value");
    this->getFunctionInformation()
        ->getBasicBlockInformation(&B)
        ->addRemoveAlloc(errBlock, const_cast<UniqueKey *>(ele));
  }
}

BasicBlockWorkList BaseAnalyzer::getErrorValues(llvm::Instruction *I,
                                                llvm::BasicBlock &B,
                                                int errcode) {
  BasicBlockWorkList BList;
  llvm::ICmpInst *ICI = llvm::cast<llvm::ICmpInst>(I);
  llvm::Value *comVal = this->getComparedValue(ICI);

  if (llvm::isa<llvm::ConstantInt>(ICI->getOperand(1))) {
    llvm::CallInst *CI = NULL;
    STFREE_LOG(I, "Compare with Int: Error Code");
    if (auto comValCI = llvm::dyn_cast<llvm::CallInst>(comVal)) {
      CI = comValCI;
    } else {
      CI = this->getFunctionInformation()
               ->getBasicBlockInformation(&B)
               ->getCallInstForVal(comVal);
    }
    if (CI) {
      STFREE_LOG(CI, "Error Code");
      for (auto ele : this->getErrorAllocInCalledFunction(CI, errcode)) {
        BList.add(ele);
      }
      for (auto ele : this->getErrorFreeInCalledFunction(CI, errcode)) {
        BList.add(ele);
      }
    }
  } else if (llvm::isa<llvm::ConstantPointerNull>(ICI->getOperand(1))) {
    llvm::CallInst *CI = NULL;
    STFREE_LOG(I, "Compare with NULL: Error Code");
    if (auto comValCI = llvm::dyn_cast<llvm::CallInst>(comVal)) {
      CI = comValCI;
    } else {
      CI = this->getFunctionInformation()
               ->getBasicBlockInformation(&B)
               ->getCallInstForVal(comVal);
    }
    if (CI) {
      STFREE_LOG(CI, "Error Code");
      for (auto ele : this->getErrorAllocInCalledFunction(CI, 0)) {
        BList.add(ele);
      }
      for (auto ele : this->getErrorFreeInCalledFunction(CI, 0)) {
        BList.add(ele);
      }
    }
    // STFREE_LOG(I, "Compare with NULL: Look at allocation", true);
    // ParentList plist = this->decodeErrorTypes(ICI->getOperand(0));
    // Type *Ty = this->getComparedType(comVal, B);
    // if (plist.size() > 0) {
    //   if (auto StTy = dyn_cast<StructType>(get_type(plist.back().first))) {
    //     if (0 <= plist.back().second &&
    //         plist.back().second < StTy->getNumElements())
    //       Ty = StTy->getElementType(plist.back().second);
    //   }
    //   // if (this->getFunctionInformation()->isAllocatedInBasicBlock(
    //   //         &B, NULL, Ty, plist.back().second)) {
    //     BList.add(
    //         this->getFunctionInformation()->getUniqueKeyManager()->getUniqueKey(
    //             NULL, Ty, plist.back().second));
    //   // }
    // }

    // // if (this->getFunctionInformation()->isAllocatedInBasicBlock(&B, NULL,
    // Ty,
    // // ROOT_INDEX)) {
    //   BList.add(
    //       this->getFunctionInformation()->getUniqueKeyManager()->getUniqueKey(
    //           NULL, Ty, ROOT_INDEX));
    // // }
  }
  return BList;
}

BasicBlockWorkList BaseAnalyzer::getErrorFreedValues(llvm::Instruction *I,
                                                     llvm::BasicBlock &B,
                                                     int errcode) {
  BasicBlockWorkList BList;
  llvm::ICmpInst *ICI = llvm::cast<llvm::ICmpInst>(I);
  llvm::Value *comVal = this->getComparedValue(ICI);

  if (llvm::isa<llvm::ConstantInt>(ICI->getOperand(1))) {
    llvm::CallInst *CI = NULL;
    STFREE_LOG(I, "Compare with Int: Error Code");
    if (auto comValCI = llvm::dyn_cast<llvm::CallInst>(comVal)) {
      CI = comValCI;
    } else {
      CI = this->getFunctionInformation()
               ->getBasicBlockInformation(&B)
               ->getCallInstForVal(comVal);
    }
    if (CI) {
      STFREE_LOG(CI, "Error Code");
      for (auto ele : this->getErrorFreeInCalledFunction(CI, errcode)) {
        BList.add(ele);
      }
    }
  } else if (llvm::isa<llvm::ConstantPointerNull>(ICI->getOperand(1))) {
    llvm::CallInst *CI = NULL;
    STFREE_LOG(I, "Compare with NULL: Error Code");
    if (auto comValCI = llvm::dyn_cast<llvm::CallInst>(comVal)) {
      CI = comValCI;
    } else {
      CI = this->getFunctionInformation()
               ->getBasicBlockInformation(&B)
               ->getCallInstForVal(comVal);
    }
    if (CI) {
      STFREE_LOG(CI, "Error Code");
      for (auto ele : this->getErrorFreeInCalledFunction(CI, 0)) {
        BList.add(ele);
      }
    }
  }
  return BList;
}

BasicBlockWorkList BaseAnalyzer::getSuccessValues(llvm::Instruction *I,
                                                  llvm::BasicBlock &B) {
  BasicBlockWorkList BList;
  llvm::ICmpInst *ICI = llvm::cast<llvm::ICmpInst>(I);
  llvm::Value *comVal = this->getComparedValue(ICI);

  if (llvm::isa<llvm::ConstantInt>(ICI->getOperand(1))) {
    llvm::CallInst *CI = NULL;
    STFREE_LOG(I, "Compare with Int: Error Code");
    if (auto comValCI = llvm::dyn_cast<llvm::CallInst>(comVal)) {
      CI = comValCI;
    } else {
      CI = this->getFunctionInformation()
               ->getBasicBlockInformation(&B)
               ->getCallInstForVal(comVal);
    }
    if (CI) {
      STFREE_LOG(CI, "Error Code");
      for (auto ele : this->getSuccessAllocInCalledFunction(CI)) {
        BList.add(ele);
      }
    }
  } else if (llvm::isa<llvm::ConstantPointerNull>(ICI->getOperand(1))) {
    llvm::CallInst *CI = NULL;
    STFREE_LOG(I, "Compare with Int: Error Code");
    if (auto comValCI = llvm::dyn_cast<llvm::CallInst>(comVal)) {
      CI = comValCI;
    } else {
      CI = this->getFunctionInformation()
               ->getBasicBlockInformation(&B)
               ->getCallInstForVal(comVal);
    }
    if (CI) {
      STFREE_LOG(CI, "Error Code");
      for (auto ele : this->getSuccessAllocInCalledFunction(CI)) {
        BList.add(ele);
      }
    }
  }
  return BList;
}

bool BaseAnalyzer::errorCodeExists(llvm::Instruction *I, llvm::BasicBlock &B,
                                   int errcode) {
  llvm::ICmpInst *ICI = llvm::cast<llvm::ICmpInst>(I);
  llvm::Value *comVal = this->getComparedValue(ICI);

  if (llvm::isa<llvm::ConstantInt>(ICI->getOperand(1))) {
    llvm::CallInst *CI = NULL;
    STFREE_LOG_ON(I, "Compare with Int: Error Code");
    if (auto comValCI = llvm::dyn_cast<llvm::CallInst>(comVal)) {
      CI = comValCI;
    } else {
      CI = this->getFunctionInformation()
               ->getBasicBlockInformation(&B)
               ->getCallInstForVal(comVal);
    }
    if (CI) {
      STFREE_LOG_ON(CI, "Error Code");
      llvm::Function *DF = getCalledFunction(CI);
      return this->getFunctionManager()
          ->getElement(DF)
          ->errorCodeLessThanExists(errcode);
    }
  } else if (llvm::isa<llvm::ConstantPointerNull>(ICI->getOperand(1))) {
    return true;
  }
  return false;
}

llvm::Value *BaseAnalyzer::getComparedValue(llvm::ICmpInst *ICI) {
  return this->decodeComparedValue(ICI->getOperand(0));
}

llvm::Value *BaseAnalyzer::decodeComparedValue(llvm::Value *V) {
  llvm::Value *comVal = V;
  if (this->getFunctionInformation()->aliasExists(comVal)) {
    llvm::Value *aliased_value =
        this->getFunctionInformation()->getAlias(comVal);
    if (auto GEle = llvm::dyn_cast<llvm::GetElementPtrInst>(aliased_value)) {
      comVal = aliased_value;
    }
  }

  if (auto LI = llvm::dyn_cast<llvm::LoadInst>(comVal)) {
    comVal = LI->getPointerOperand();
  }

  if (auto GEle = llvm::dyn_cast<llvm::GetElementPtrInst>(comVal)) {
    GEle = getRootGEle(GEle);
    comVal = getLoadeeValue(GEle->getPointerOperand());
  }
  return comVal;
}

ParentList BaseAnalyzer::decodeErrorTypes(llvm::Value *V) {
  ParentList plist;
  llvm::Value *comVal = V;
  if (this->getFunctionInformation()->aliasExists(comVal)) {
    llvm::Value *aliased_value =
        this->getFunctionInformation()->getAlias(comVal);
    if (auto GEle = llvm::dyn_cast<llvm::GetElementPtrInst>(aliased_value)) {
      comVal = aliased_value;
    }
  }
  if (auto LI = llvm::dyn_cast<llvm::LoadInst>(comVal)) {
    comVal = LI->getPointerOperand();
  }
  if (auto BCI = llvm::dyn_cast<llvm::BitCastInst>(comVal)) {
    comVal = BCI->getOperand(0);
  }

  if (auto GEle = llvm::dyn_cast<llvm::GetElementPtrInst>(comVal)) {
    this->getStructParents(GEle, plist);
  }
  return plist;
}

llvm::Type *BaseAnalyzer::getComparedType(llvm::Value *comVal,
                                          llvm::BasicBlock &B) {
  llvm::Type *Ty = comVal->getType();
  if (this->getFunctionInformation()
          ->getBasicBlockInformation(&B)
          ->isCallValues(comVal)) {
    if (auto Alloca = llvm::dyn_cast<llvm::AllocaInst>(comVal)) {
      Ty = Alloca->getAllocatedType();
    }
  }

  if (auto CI = llvm::dyn_cast<llvm::CallInst>(comVal)) {
    Ty = CI->getType();
    for (auto usr : CI->users()) {
      if (auto CastI = llvm::dyn_cast<llvm::CastInst>(usr))
        Ty = CastI->getDestTy();
    }
  }
  return Ty;
}

int BaseAnalyzer::getErrorOperand(llvm::ICmpInst *ICI) {
  int operand = -1;
  if (auto ConstI = llvm::dyn_cast<llvm::ConstantInt>(ICI->getOperand(1))) {
    if (ConstI->isZero()) {
      // TODO: check for each case
      if (ICI->getPredicate() == llvm::CmpInst::ICMP_EQ ||
          ICI->getPredicate() == llvm::CmpInst::ICMP_SGE)
        operand = 1;
      else if (ICI->getPredicate() == llvm::CmpInst::ICMP_NE ||
               ICI->getPredicate() == llvm::CmpInst::ICMP_SLT)
        operand = 0;
    }
  } else if (llvm::isa<llvm::ConstantPointerNull>(ICI->getOperand(1))) {
    if (ICI->getPredicate() == llvm::CmpInst::ICMP_EQ)
      operand = 0;
    else if (ICI->getPredicate() == llvm::CmpInst::ICMP_NE)
      operand = 1;
  }
  return operand;
}

BasicBlockList BaseAnalyzer::getErrorAllocInCalledFunction(llvm::CallInst *CI,
                                                           int errcode) {
  llvm::Function *DF = getCalledFunction(CI);
  STFREE_LOG(CI, "Called Error");
  return this->getFunctionManager()->getElement(DF)->getAllocatedInError(
      errcode);
}

BasicBlockList BaseAnalyzer::getSuccessAllocInCalledFunction(
    llvm::CallInst *CI) {
  llvm::Function *DF = getCalledFunction(CI);
  STFREE_LOG(CI, "Called Success");
  return this->getFunctionManager()->getElement(DF)->getAllocatedInSuccess();
}

BasicBlockList BaseAnalyzer::getErrorFreeInCalledFunction(llvm::CallInst *CI,
                                                          int errcode) {
  llvm::Function *DF = getCalledFunction(CI);
  STFREE_LOG_ON(CI, "Called Error Free In Called Function");
  return this->getFunctionManager()->getElement(DF)->getFreedInError(errcode);
}

BasicBlockList BaseAnalyzer::getSuccessFreeInCalledFunction(
    llvm::CallInst *CI) {
  llvm::Function *DF = getCalledFunction(CI);
  STFREE_LOG(CI, "Called Success Free In Called Function");
  return this->getFunctionManager()->getElement(DF)->getFreedInSuccess();
}

void BaseAnalyzer::buildReturnValueInformation() {
  llvm::ReturnInst *RI = getFunctionInformation()->getReturnInst();
  llvm::BasicBlock *B = getFunctionInformation()->getEndPoint();

  llvm::Type *RetTy = getFunctionInformation()->getFunction().getReturnType();
  if (RetTy->isIntegerTy()) {
    if (RI->getNumOperands() <= 0) return;
    llvm::Value *V = RI->getReturnValue();
    this->checkErrorInstruction(V);
  } else if (RetTy->isPointerTy()) {
    // TODO: add support to pointers
    STFREE_LOG(RI, "[RETURN]: No Error Code Analysis");
    getFunctionInformation()->addSuccessBlockInformation(B);
  } else {
    STFREE_LOG(RI, "[RETURN]: No Error Code Analysis");
    getFunctionInformation()->addSuccessBlockInformation(B);
  }
}

void BaseAnalyzer::checkErrorCodeAndAddBlock(
    llvm::Instruction *I, llvm::BasicBlock *B, llvm::Value *inval,
    std::vector<llvm::Instruction *> visited_inst) {
  if (auto CInt = llvm::dyn_cast<llvm::ConstantInt>(inval)) {
    STFREE_LOG_ON(I, "Storing constant value to ret");
    int64_t errcode = CInt->getSExtValue();
    if (errcode < NO_ERROR) {
      STFREE_LOG_ON(I, "[RETURN] ERR: " + std::to_string(errcode));
      getFunctionInformation()->addErrorBlockInformation(errcode, B);
    } else {
      STFREE_LOG_ON(I, "[RETURN] SUCCESS: " + std::to_string(errcode));
      getFunctionInformation()->addSuccessBlockInformation(B);
    }
  } else if (auto CI = llvm::dyn_cast<llvm::CallInst>(inval)) {
    STFREE_LOG(I, "Storing caall inst value to ret");
    if (FunctionInformation *DF =
            getFunctionManager()->getElement(getCalledFunction(CI))) {
      for (auto err_code_info : DF->getErrorCodeMap()) {
        int64_t errcode = err_code_info.first;

        if (errcode < NO_ERROR) {
          STFREE_LOG(I, "[RETURN][CALLINST] ERR: " + std::to_string(errcode));
          getFunctionInformation()->addErrorBlockFreeInformation(
              errcode, err_code_info.second.free_list);
          getFunctionInformation()->addErrorBlockAllocInformation(
              errcode, err_code_info.second.alloc_list);
        } else {
          STFREE_LOG(I,
                     "[RETURN][CALLINST] SUCCESS: " + std::to_string(errcode));
          getFunctionInformation()->addErrorBlockFreeInformation(
              0, err_code_info.second.free_list);
          getFunctionInformation()->addErrorBlockAllocInformation(
              0, err_code_info.second.alloc_list);
        }
      }
    }
  } else if (inval->getType()->isPointerTy()) {
    if (llvm::isa<llvm::ConstantPointerNull>(inval)) {
      STFREE_LOG(I, "[RETURN] ERR: NULL");
      getFunctionInformation()->addErrorBlockInformation(-1, B);
    } else {
      STFREE_LOG(I, "[RETURN] SUCCESS: Non-NULL");
      getFunctionInformation()->addSuccessBlockInformation(B);
    }
  } else {
    if (auto PHI = llvm::dyn_cast<llvm::PHINode>(inval)) {
      STFREE_LOG(PHI, "[ERRORINST]: PHINode Instruction Reivisted");
      if (find(visited_inst.begin(), visited_inst.end(), PHI) ==
          visited_inst.end()) {
        visited_inst.push_back(PHI);
        checkErrorInstruction(PHI, visited_inst);
      }
    } else if (auto SelI = llvm::dyn_cast<llvm::SelectInst>(inval)) {
      STFREE_LOG_ON(SelI, "[ERRORINST][SELECT]: Select instruction");
      if (find(visited_inst.begin(), visited_inst.end(), SelI) ==
          visited_inst.end()) {
        visited_inst.push_back(SelI);
        checkErrorInstruction(SelI->getTrueValue(), visited_inst);
        checkErrorInstruction(SelI->getFalseValue(), visited_inst);
      }
    }
  }
  return;
}

void BaseAnalyzer::checkErrorInstruction(
    llvm::Value *V, std::vector<llvm::Instruction *> visited_inst) {
  if (auto CInt = llvm::dyn_cast<llvm::ConstantInt>(V)) {
    if (visited_inst.size() > 0) {
      llvm::BasicBlock *B = visited_inst.back()->getParent();
      int errcode = CInt->getSExtValue();
      if (errcode < NO_ERROR) {
        STFREE_LOG_ON(visited_inst.back(),
                      "[RETURN] ERR: " + std::to_string(errcode));
        getFunctionInformation()->addErrorBlockInformation(errcode, B);
      } else {
        STFREE_LOG_ON(visited_inst.back(),
                      "[RETURN] SUCCESS: " + std::to_string(errcode));
        getFunctionInformation()->addSuccessBlockInformation(B);
      }
    }
  }
  if (auto CI = llvm::dyn_cast<llvm::CallInst>(V)) {
    STFREE_LOG(CI, "[ERRORINST]: Call Inst");
    if (getCalledFunction(CI)) {
      STFREE_LOG(CI, CI->getFunction()->getName());
    }
  }
  if (auto LI = llvm::dyn_cast<llvm::LoadInst>(V)) {
    STFREE_LOG_ON(LI, "[ERRORINST]: Load Instruction");
    for (auto usr : LI->getPointerOperand()->users()) {
      if (auto SI = llvm::dyn_cast<llvm::StoreInst>(usr)) {
        if (V != SI->getValueOperand())
          this->checkErrorCodeAndAddBlock(SI, SI->getParent(),
                                          SI->getValueOperand(), visited_inst);
      }
    }
  } else if (auto PHI = llvm::dyn_cast<llvm::PHINode>(V)) {
    STFREE_LOG_ON(PHI, "[ERRORINST]: PHINode Instruction " +
                           std::to_string(PHI->getNumIncomingValues()));
    for (unsigned i = 0; i < PHI->getNumIncomingValues(); i++) {
      if (V != PHI->getIncomingValue(i))
        this->checkErrorCodeAndAddBlock(PHI, PHI->getIncomingBlock(i),
                                        PHI->getIncomingValue(i), visited_inst);
    }
  } else if (auto IcmpI = llvm::dyn_cast<llvm::ICmpInst>(V)) {
    // Might be switch null check. Check.
  }
  return;
}

bool BaseAnalyzer::isBidirectionalAlias(llvm::Value *V) {
  if (llvm::Value *aliasVal = getFunctionInformation()->getAlias(V)) {
    if (llvm::Instruction *I = llvm::dyn_cast<llvm::Instruction>(aliasVal)) {
      if (isStructEleFree(I)) {
        struct collectedInfo info;
        ParentList plist;
        this->collectStructMemberFreeInfo(I, info, plist);
        if (info.freeValue == V) return true;
      }
    }
  }
  return false;
}

void BaseAnalyzer::reversePropagateErrorBlockFreeInfo() {
  // for (auto err : this->getFunctionInformation()->getErrorBlock()) {
  //   for (BasicBlock *preds : predecessors(err.second)) {
  //     if (this->getFunctionInformation()->getBasicBlockInformation(preds) &&
  //         this->getFunctionInformation()
  //             ->getBasicBlockInformation(preds)
  //             ->isErrorHandlingBlock())
  //       __recursiveReversePropagateErrorBlockFreeInfo(preds);
  //   }
  // }
  // for (BasicBlock *retBlock :
  // this->getFunctionInformation()->getEndPoint())
  // {
  //     for(BasicBlock *preds: predecessors(retBlock))
  //         if
  //         (this->getFunctionInformation()->getBasicBlockInformation(preds)->isErrorHandlingBlock())
  //             __recursiveReversePropagateErrorBlockFreeInfo(preds);
  // }
  return;
}

void BaseAnalyzer::__recursiveReversePropagateErrorBlockFreeInfo(
    llvm::BasicBlock *B) {
  for (llvm::BasicBlock *preds : llvm::predecessors(B)) {
    if (this->getFunctionInformation()
            ->getBasicBlockInformation(preds)
            ->isErrorHandlingBlock() &&
        !this->getFunctionInformation()
             ->getBasicBlockInformation(preds)
             ->isReversePropagated()) {
      this->getFunctionInformation()->getBasicBlockManager()->copyFreed(B,
                                                                        preds);
      this->getFunctionInformation()
          ->getBasicBlockInformation(preds)
          ->setReversePropagated();
      __recursiveReversePropagateErrorBlockFreeInfo(preds);
    }
  }
  return;
}

bool BaseAnalyzer::isCallInstReturnValue(llvm::Value *V) {
  llvm::Value *tgt_val = V;
  if (auto LI = llvm::dyn_cast<llvm::LoadInst>(tgt_val)) {
    tgt_val = LI->getPointerOperand();
  }

  if (auto CI = llvm::dyn_cast<llvm::CallInst>(tgt_val)) return true;

  return false;
}

bool BaseAnalyzer::isAllocStoredInSameBasicBlock(llvm::Value *V,
                                                 llvm::BasicBlock *B) {
  llvm::Value *stored_value = V;
  for (auto user : V->users()) {
    if (auto SI = llvm::dyn_cast<llvm::StoreInst>(user)) {
      if (SI->getParent() == B) {
        return true;
      }
    }
  }
  return false;
}

llvm::Function *BaseAnalyzer::getCalledFunction(llvm::CallInst *CI) {
  llvm::Function *called_func = NULL;

  if (auto normal_called_func = CI->getCalledFunction())
    called_func = normal_called_func;

  if (auto *ConstExpr =
          llvm::dyn_cast<llvm::ConstantExpr>(CI->getCalledOperand())) {
    if (ConstExpr->isCast()) {
      if (auto *func =
              llvm::dyn_cast<llvm::Function>(ConstExpr->getOperand(0))) {
        called_func = func;
      }
    }
  }

  return called_func;
}

std::pair<llvm::BasicBlock *, int> BaseAnalyzer::getLastUseBlock(
    llvm::Instruction *I, std::vector<llvm::Instruction *> visited_values,
    int max_itr) {
  std::pair<llvm::BasicBlock *, int> candidate =
      std::pair<llvm::BasicBlock *, int>(I->getParent(), visited_values.size());

  if (std::find(visited_values.begin(), visited_values.end(), I) ==
          visited_values.end() &&
      max_itr-- >= 0) {
    visited_values.push_back(I);
    for (auto user : I->users()) {
      if (auto next_inst = llvm::dyn_cast<llvm::Instruction>(user)) {
        std::pair<llvm::BasicBlock *, int> target_cand =
            getLastUseBlock(next_inst, visited_values, max_itr);
        if (target_cand.second > candidate.second) {
          candidate = target_cand;
        }
      }
    }
  }

  return candidate;
}

}  // namespace ST_free
