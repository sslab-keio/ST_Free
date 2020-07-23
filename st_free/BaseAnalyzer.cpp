#include "include/BaseAnalyzer.hpp"

#define isEntryPoint(F, B) &(F.getEntryBlock()) == &B ? true : false
#define UpdateIfNull(tgt, cand) (tgt) = ((tgt) == NULL ? (cand) : (tgt))

namespace ST_free {
void BaseAnalyzer::analyzeAdditionalUnknowns(Function &F) {
  for (BasicBlock &B : F) {
    for (Instruction &I : B) {
      if (StoreInst *SI = dyn_cast<StoreInst>(&I))
        this->checkAndChangeActualAuthority(SI);
    }
  }
  return;
}

void BaseAnalyzer::analyze(Function &F) {
  setFunctionInformation(identifier.getElement(&F));

  if (!getFunctionInformation()->isUnanalyzed()) return;

  int iterate_counter = 0;
  do {
    getFunctionInformation()->setInProgress();
    for (BasicBlock &B : F) {
      generateWarning(B.getFirstNonPHI(), B.getName());
      getFunctionInformation()->BBCollectInfo(B, isEntryPoint(F, B));
      this->analyzeInstructions(B);
      getFunctionInformation()->updateSuccessorBlock(B);
      getFunctionInformation()->getBasicBlockManager()->shrinkFreedFromAlloc(
          &B);
      // generateWarning(B.getFirstNonPHI(),
      //                 "Free: " + to_string(getFunctionInformation()
      //                                          ->getBasicBlockManager()
      //                                          ->getBasicBlockFreeList(&B)
      //                                          .size()),
      //                 true);
      // generateWarning(B.getFirstNonPHI(),
      //                 "Alloc: " + to_string(getFunctionInformation()
      //                                           ->getBasicBlockManager()
      //                                           ->getBasicBlockAllocList(&B)
      //                                           .size()),
      //                 true);
      if (!getFunctionInformation()
               ->getBasicBlockInformation(&B)
               ->isInformationIdenticalToBackup()) {
        generateWarning(B.getFirstNonPHI(), "Information not Identical");
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

void BaseAnalyzer::analyzeInstructions(BasicBlock &B) {
  for (Instruction &I : B) {
    if (InstAnalysisMap.find(I.getOpcode()) != InstAnalysisMap.end())
      (this->*InstAnalysisMap[I.getOpcode()])(&I, B);
  }
}

void BaseAnalyzer::analyzeAllocaInst(Instruction *AI, BasicBlock &B) {}

void BaseAnalyzer::analyzeICmpInst(Instruction *I, BasicBlock &B) {}

void BaseAnalyzer::analyzeStoreInst(Instruction *I, BasicBlock &B) {
  StoreInst *SI = cast<StoreInst>(I);
  if (this->isStoreToStructMember(SI)) {
    generateWarning(SI, "is Store to struct");
    GetElementPtrInst *GEle = getStoredStruct(SI);
    stManage->addStore(cast<StructType>(GEle->getSourceElementType()),
                       getValueIndices(GEle).back());

    if (isa<GlobalValue>(SI->getValueOperand())) {
      generateWarning(SI, "GolbalVariable Store");
      stManage->addGlobalVarStore(
          cast<StructType>(GEle->getSourceElementType()),
          getValueIndices(GEle).back());
    }

    if (isa<AllocaInst>(SI->getValueOperand())) {
      getFunctionInformation()->setAlias(GEle, SI->getValueOperand());
    }
  }

  if (this->isStoreFromStructMember(SI)) {
    generateWarning(SI, "is Store from struct");
    GetElementPtrInst *GEle = getStoredStructEle(SI);
    if (isa<AllocaInst>(SI->getPointerOperand())) {
      getFunctionInformation()->setAlias(GEle, SI->getPointerOperand());
    }
  }
}

void BaseAnalyzer::analyzeCallInst(Instruction *I, BasicBlock &B) {
  CallInst *CI = cast<CallInst>(I);

  if (Function *called_function = CI->getCalledFunction()) {
    if (isAllocFunction(called_function)) {
      Value *val = getAllocatedValue(CI);
      if (val != NULL)
        if (StructType *strTy =
                dyn_cast<StructType>(get_type(val->getType()))) {
          stManage->addAlloc(strTy);
        }
      // this->addAlloc(CI, &B);
    } else if (isFreeFunction(called_function)) {
      for (auto arguments = CI->arg_begin(); arguments != CI->arg_end();
           arguments++) {
        this->addFree(cast<Value>(arguments), CI, &B);
      }
    } else {
      this->analyzeDifferentFunc((Function &)(*called_function));
      this->copyArgStatus((Function &)(*called_function), CI, B);
    }
  }
}

void BaseAnalyzer::analyzeBranchInst(Instruction *I, BasicBlock &B) {
  BranchInst *BI = cast<BranchInst>(I);
  if (this->isCorrectlyBranched(BI)) {
    generateWarning(BI, "Correctly Branched");
    getFunctionInformation()->setCorrectlyBranched(&B);
  }
}

void BaseAnalyzer::analyzeBitCastInst(Instruction *I, BasicBlock &B) {
  BitCastInst *BCI = cast<BitCastInst>(I);

  Type *tgtTy = get_type(BCI->getDestTy());
  if (get_type(BCI->getSrcTy())->isIntegerTy()) {
    if (tgtTy->isStructTy()) {
      Value *V = BCI->getOperand(0);
      getFunctionInformation()->addAliasedType(V, tgtTy);
    }
  }

  if (tgtTy->isIntegerTy()) {
    Type *srcTy = get_type(BCI->getSrcTy());
    if (srcTy->isStructTy()) {
      Value *V = BCI->getOperand(0);
      getFunctionInformation()->addAliasedType(V, srcTy);
    }
  }
  return;
}

void BaseAnalyzer::analyzeReturnInst(Instruction *I, BasicBlock &B) {
  ReturnInst *RI = cast<ReturnInst>(I);

  getFunctionInformation()->addEndPoint(&B, RI);
  getFunctionInformation()->clearErrorCodeMap();

  Type *RetTy = getFunctionInformation()->getFunction().getReturnType();
  if (RetTy->isIntegerTy()) {
    if (RI->getNumOperands() <= 0) return;
    Value *V = RI->getReturnValue();
    this->checkErrorInstruction(V);
  } else if (RetTy->isPointerTy()) {
    // TODO: add support to pointers
    generateWarning(RI, "[RETURN][POINTER]: No Error Code Analysis");
    Value *V = RI->getReturnValue();
    this->checkErrorInstruction(V);
    // getFunctionInformation()->addSuccessBlockInformation(&B);
  } else {
    generateWarning(RI, "[RETURN]: No Error Code Analysis");
    getFunctionInformation()->addSuccessBlockInformation(&B);
  }
  return;
}

void BaseAnalyzer::analyzeGetElementPtrInst(Instruction *I, BasicBlock &B) {
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
    StructType *strTy = cast<StructType>(freedStruct->getType());
    int cPointers = strTy->getNumElements();
    vector<bool> alreadyFreed = freedStruct->getFreedMember();
    for (int ind = 0; ind < strTy->getNumElements(); ind++) {
      Type *t = strTy->getElementType(ind);
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

bool BaseAnalyzer::isCorrectlyBranched(BranchInst *BI) {
  if (BI->isConditional() && BI->getCondition() != NULL) {
    if (auto *CmpI = dyn_cast<CmpInst>(BI->getCondition())) {
      if (auto *LI = dyn_cast<LoadInst>(CmpI->getOperand(0)))
        if (string(LI->getPointerOperand()->getName()).find("ref") !=
            string::npos)
          return true;
      if (isa<ConstantPointerNull>(CmpI->getOperand(1))) {
        return true;
      }
    }
  }
  return false;
}

void BaseAnalyzer::addLocalVariable(BasicBlock *B, Type *T, Value *V,
                                    Instruction *I, ParentList P) {
  ValueInformation *vinfo = getFunctionInformation()->addVariable(V);
  getFunctionInformation()->addBasicBlockLiveVariable(B, V);
  if (StructType *strTy = dyn_cast<StructType>(T)) {
    getFunctionInformation()->addLocalVar(B, strTy, V, I, P, vinfo);

    // P.push_back(T);
    for (Type *ele : strTy->elements()) {
      if (ele->isStructTy()) this->addLocalVariable(B, ele, V, I, P);
    }
  }
  return;
}

void BaseAnalyzer::addPointerLocalVariable(BasicBlock *B, Type *T, Value *V,
                                           Instruction *I, ParentList P) {
  ValueInformation *vinfo = getFunctionInformation()->addVariable(V);
  if (StructType *strTy = dyn_cast<StructType>(get_type(T))) {
    getFunctionInformation()->addLocalVar(B, strTy, V, I, P, vinfo);

    // P.push_back(T);
    for (Type *ele : strTy->elements()) {
      if (ele->isStructTy()) this->addLocalVariable(B, ele, V, I, P);
    }
  }
  return;
}

void BaseAnalyzer::analyzeDifferentFunc(Function &F) {
  /*** Push current FunctionInformation ***/
  functionStack.push(&getFunctionInformation()->getFunction());

  /*** Analyze new Function ***/
  this->analyze(F);

  /*** Recover FunctionInformation ***/
  Function *tempFunc = functionStack.top();
  setFunctionInformation(identifier.getElement(tempFunc));
  functionStack.pop();
  return;
}

void BaseAnalyzer::addFree(Value *V, CallInst *CI, BasicBlock *B, bool isAlias,
                           ParentList additionalParents) {
  struct collectedInfo info;

  if (Instruction *val = dyn_cast<Instruction>(V)) {
    if (isStructEleFree(val) || additionalParents.size() > 0) {
      generateWarning(CI, "Struct Element Free");
      this->collectStructMemberFreeInfo(val, info, additionalParents);
    }

    if (isStructFree(val)) {
      generateWarning(CI, "Struct Free");
      this->collectStructFreeInfo(val, info);
    } else if (isOptimizedStructFree(val)) {
      generateWarning(CI, "Optimized Struct Free");
      this->collectOptimizedStructFreeInfo(val, info);
    }

    if (!info.isStructRelated) this->collectSimpleFreeInfo(val, info);
  } else {
    // When falling into this branch, it is either a value is optimized arg or
    // it is something else (not really sure). We need to decode this by
    // ourselves This is a temporary implementation.
    // TODO: fix this to more stable implementation.
    generateWarning(CI, "Non Instruction free value found");

    // Get Top-level Value/Type
    Type *Ty = V->getType();
    for (auto user : V->users()) {
      if (auto BCI = dyn_cast<BitCastInst>(user)) {
        Ty = BCI->getDestTy();
      }
    }

    // If additionalParents exists, this means that it is a struct member.
    // Decode everything back.
    if (additionalParents.size() > 0) {
      generateWarning(CI, "reading from additional parents");
      info.indexes = additionalParents;
      info.index = additionalParents.back().second;

      if (auto StTy =
              dyn_cast<StructType>(get_type(info.indexes.back().first))) {
        if (0 <= info.index && info.index < StTy->getNumElements())
          UpdateIfNull(info.memType, StTy->getElementType(info.index));
      }
    }

    UpdateIfNull(info.memType, Ty);
    UpdateIfNull(info.freeValue, V);
  }

  if (info.freeValue && !getFunctionInformation()->isFreedInBasicBlock(
                            B, info.freeValue, info.memType, info.index)) {
    generateWarning(CI, "Adding Free Value", true);
    ValueInformation *valInfo = getFunctionInformation()->addFreeValue(
        B, NULL, info.memType, info.index, info.indexes);

    if (getFunctionInformation()->isArgValue(info.freeValue)) {
      generateWarning(CI, "Add Free Arg");
      valInfo->setArgNumber(
          getFunctionInformation()->getArgIndex(info.freeValue));
      if (!info.parentType)
        getFunctionInformation()->setArgFree(info.freeValue);
      else if (info.parentType && info.index >= 0) {
        generateWarning(CI, "parentType add free arg");
        getFunctionInformation()->setStructMemberArgFreed(info.freeValue,
                                                          info.indexes);
      }
    }

    if (!isAlias && !getFunctionInformation()->aliasExists(info.freeValue) &&
        info.memType && get_type(info.memType)->isStructTy() &&
        this->isAuthorityChained(info.indexes)) {
      generateWarning(CI, "Add Freed Struct");
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
      Value *aliasVal = getFunctionInformation()->getAlias(info.freeValue);
      if (V != aliasVal) this->addFree(aliasVal, CI, B, true);
    }
  }
}

void BaseAnalyzer::addAlloc(CallInst *CI, BasicBlock *B) {
  Type *Ty = CI->getType();
  for (User *usr : CI->users()) {
    if (auto CastI = dyn_cast<CastInst>(usr)) {
      Ty = CastI->getDestTy();
    } else if (auto SI = dyn_cast<StoreInst>(usr)) {
      generateWarning(CI, "struct store");
    }
  }
  if (get_type(Ty)->isStructTy()) {
    getFunctionInformation()->addAliasedType(CI, Ty);
    getStructManager()->addAlloc(cast<StructType>(get_type(Ty)));
  }
  getFunctionInformation()->addAllocValue(B, NULL, Ty, ROOT_INDEX);
  getFunctionInformation()->addAllocValue(B, CI, Ty, ROOT_INDEX);
  return;
}

bool BaseAnalyzer::isReturnFunc(Instruction *I) {
  if (isa<ReturnInst>(I)) return true;
  return false;
}

void BaseAnalyzer::copyArgStatus(Function &Func, CallInst *CI, BasicBlock &B) {
  FunctionInformation *DF = identifier.getElement(&Func);
  int ind = 0;

  for (auto arguments = CI->arg_begin(); arguments != CI->arg_end();
       arguments++, ind++) {
    ArgStatus *args = DF->getArgList()->getArgStatus(ind);
    this->copyArgStatusRecursively(Func, CI, B, cast<Value>(arguments), args,
                                   ind, NULL, ParentList(), true);
  }
  return;
}

void BaseAnalyzer::copyArgStatusRecursively(Function &Func, CallInst *CI,
                                            BasicBlock &B, Value *arg,
                                            ArgStatus *ArgStat, int ind,
                                            Type *ParentType, ParentList plist,
                                            bool isFirst) {
  if (ArgStat && ArgStat->isStruct()) {
    generateWarning(CI, "Args is Struct");
    if (!isFirst) plist.push_back(pair<Type *, int>(ParentType, ind));

    if (ArgStat->isFreed()) {
      generateWarning(CI, "Copy Args Stat");
      this->addFree(arg, CI, &B, false, plist);
      Type *T = get_type(ArgStat->getType());
      if (isa<StructType>(T))
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

void BaseAnalyzer::copyAllocatedStatus(Function &Func, BasicBlock &B) {
  FunctionInformation *DF = identifier.getElement(&Func);
  for (auto ele : DF->getAllocatedInReturn()) {
    getFunctionInformation()->addAllocValue(&B, const_cast<UniqueKey *>(ele));
  }
  return;
}

void BaseAnalyzer::copyFreeStatus(Function &Func, CallInst *CI, BasicBlock &B) {
  FunctionInformation *DF = identifier.getElement(&Func);
  generateWarning(CI, "Copy Free Status", true);
  for (auto ele : DF->getFreedInSuccess()) {
    generateWarning(
        CI, "Copying Free Status " + to_string(DF->getFreedInSuccess().size()),
        true);
    if (ValueInformation *vinfo = DF->getValueInfo(ele)) {
      generateWarning(CI, "Getting Value Info");
      if (vinfo->isArgValue()) {
        generateWarning(CI, "Copying value");
        if (vinfo->getArgNumber() < CI->getNumArgOperands())
          addFree(CI->getArgOperand(vinfo->getArgNumber()), CI, &B, false,
                  vinfo->getParents());
      }
    }
  }
  return;
}

void BaseAnalyzer::evaluatePendingStoredValue(Function &Func, CallInst *CI,
                                              BasicBlock &B) {
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

CallInst *BaseAnalyzer::getStoreFromCall(StoreInst *SI) {
  Value *val = SI->getValueOperand();
  if (auto BCI = dyn_cast<CastInst>(val)) {
    val = BCI->getOperand(0);
  }
  if (CallInst *CI = dyn_cast<CallInst>(val)) return CI;
  return NULL;
}

bool BaseAnalyzer::isStoreToStructMember(StoreInst *SI) {
  Value *v = SI->getPointerOperand();
  if (auto BCI = dyn_cast<CastInst>(v)) v = BCI->getOperand(0);
  if (GetElementPtrInst *gEle = dyn_cast<GetElementPtrInst>(v)) {
    return true;
  }
  return false;
}

bool BaseAnalyzer::isStoreFromStructMember(StoreInst *SI) {
  if (getStoredStructEle(SI)) return true;
  return false;
}

bool BaseAnalyzer::isStoreToStruct(StoreInst *SI) {
  Type *Ty = SI->getPointerOperandType();
  if (auto CI = dyn_cast<CastInst>(SI->getPointerOperand())) {
    Ty = CI->getSrcTy();
  }

  if (Ty->isPointerTy()) {
    if (get_type(Ty)->isStructTy()) return true;
  }
  return false;
}

StructType *BaseAnalyzer::getStoreeStruct(StoreInst *SI) {
  Type *Ty = SI->getPointerOperandType();
  if (auto CI = dyn_cast<CastInst>(SI->getPointerOperand())) {
    Ty = CI->getSrcTy();
  }

  if (Ty->isPointerTy()) {
    if (auto StTy = dyn_cast<StructType>(get_type(Ty))) return StTy;
  }
  return NULL;
}

StructType *BaseAnalyzer::getStorerStruct(StoreInst *SI) {
  Type *Ty = SI->getValueOperand()->getType();
  if (auto CI = dyn_cast<CastInst>(SI->getValueOperand())) {
    Ty = CI->getSrcTy();
  }

  if (Ty->isPointerTy()) {
    if (auto StTy = dyn_cast<StructType>(get_type(Ty))) return StTy;
  }
  return NULL;
}

bool BaseAnalyzer::isStoreFromStruct(StoreInst *SI) {
  if (get_type(SI->getValueOperand()->getType())->isStructTy()) return true;
  return false;
}

void BaseAnalyzer::checkAndChangeActualAuthority(StoreInst *SI) {
  vector<CastInst *> CastInsts;
  if (this->isStoreToStructMember(SI)) {
    GetElementPtrInst *GEle = getStoredStruct(SI);
    if (GEle && isa<StructType>(GEle->getSourceElementType())) {
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

      for (auto CI : CastInsts) this->changeAuthority(SI, CI, GEle);
    }
  }
}

void BaseAnalyzer::changeAuthority(StoreInst *SI, CastInst *CI,
                                   GetElementPtrInst *GEle) {
  ParentList indexes;
  generateWarning(SI, "is Casted Store");
  this->getStructParents(GEle, indexes);

  if (indexes.size() > 0 &&
      this->isAuthorityChained(
          vector<pair<Type *, int>>(indexes.end() - 1, indexes.end())) &&
      !this->isAllocCast(CI) && !this->isCastToVoid(CI)) {
    if (StructType *StTy = dyn_cast<StructType>(indexes.back().first)) {
      generateWarning(SI, "Change back to Unknown");
      getStructManager()->get(StTy)->setMemberStatUnknown(
          indexes.back().second);
    }
  }
  return;
}

vector<string> BaseAnalyzer::decodeDirectoryName(string fname) {
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

void BaseAnalyzer::getStructParents(Instruction *I,
                                    vector<pair<Type *, int>> &typeList) {
  if (LoadInst *LI = dyn_cast<LoadInst>(I)) {
    if (Instruction *Inst = dyn_cast<Instruction>(LI->getPointerOperand()))
      this->getStructParents(Inst, typeList);
  } else if (GetElementPtrInst *GI = dyn_cast<GetElementPtrInst>(I)) {
    if (Instruction *Inst = dyn_cast<Instruction>(GI->getPointerOperand()))
      this->getStructParents(Inst, typeList);
    vector<pair<Type *, long>> decoded_vals = this->decodeGEPInst(GI);
    for (auto dec : decoded_vals) {
      if (dec.first != NULL && dec.second != ROOT_INDEX)
        typeList.push_back(pair<Type *, int>(dec.first, dec.second));
    }
  }
  return;
}

long BaseAnalyzer::getMemberIndiceFromByte(StructType *STy, uint64_t byte) {
  const StructLayout *sl = this->getStructLayout(STy);
  if (sl != NULL) return sl->getElementContainingOffset(byte);
  return ROOT_INDEX;
}

vector<long> BaseAnalyzer::getValueIndices(GetElementPtrInst *inst) {
  long indice = ROOT_INDEX;
  vector<long> indices;

  Type *Ty = inst->getSourceElementType();
  auto idx_itr = inst->idx_begin();
  if (!Ty->isIntegerTy()) idx_itr++;

  // for(auto idx_itr = inst->idx_begin() + 1; idx_itr != inst->idx_end();
  // idx_itr++) {
  for (; idx_itr != inst->idx_end(); idx_itr++) {
    if (ConstantInt *cint = dyn_cast<ConstantInt>(idx_itr->get()))
      indice = cint->getSExtValue();
    else
      indice = ROOT_INDEX;
    indices.push_back(indice);
  }

  return indices;
}

GetElementPtrInst *BaseAnalyzer::getRootGEle(GetElementPtrInst *GEle) {
  GetElementPtrInst *tgt = GEle;
  while (isa<GetElementPtrInst>(tgt->getPointerOperand())) {
    tgt = cast<GetElementPtrInst>(tgt->getPointerOperand());
  }
  return tgt;
}

bool BaseAnalyzer::isStructEleAlloc(Instruction *val) {
  for (User *usr : val->users()) {
    User *tmp_usr = usr;
    if (!isa<StoreInst>(usr)) {
      for (User *neo_usr : usr->users()) {
        if (isa<StoreInst>(neo_usr)) {
          tmp_usr = neo_usr;
          break;
        }
      }
    }
    if (StoreInst *str_inst = dyn_cast<StoreInst>(tmp_usr)) {
      Value *tgt_op = str_inst->getOperand(1);
      if (GetElementPtrInst *inst = dyn_cast<GetElementPtrInst>(tgt_op)) {
        return true;
      }
    }
  }
  return false;
}

Value *BaseAnalyzer::getAllocatedValue(Instruction *val) {
  for (User *usr : val->users()) {
    User *tmp_usr = usr;
    if (!isa<StoreInst>(usr)) {
      for (User *neo_usr : usr->users()) {
        if (isa<StoreInst>(neo_usr)) {
          tmp_usr = neo_usr;
          break;
        }
      }
    }
    if (StoreInst *str_inst = dyn_cast<StoreInst>(tmp_usr)) {
      Value *tgt_op = str_inst->getOperand(1);
      return tgt_op;
    }
  }
  return NULL;
}

GetElementPtrInst *BaseAnalyzer::getAllocStructEleInfo(Instruction *val) {
  for (User *usr : val->users()) {
    User *tmp_usr = usr;
    if (!isa<StoreInst>(usr)) {
      for (User *neo_usr : usr->users()) {
        if (isa<StoreInst>(neo_usr)) {
          tmp_usr = neo_usr;
          break;
        }
      }
    }
    if (StoreInst *str_inst = dyn_cast<StoreInst>(tmp_usr)) {
      Value *tgt_op = str_inst->getOperand(1);
      if (GetElementPtrInst *inst = dyn_cast<GetElementPtrInst>(tgt_op)) {
        return inst;
      }
    }
  }
  return NULL;
}

bool BaseAnalyzer::isStructEleFree(Instruction *val) {
  if (isa<GetElementPtrInst>(val)) return true;

  LoadInst *l_inst = find_load(val);
  if (l_inst && l_inst->getOperandList()) {
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

GetElementPtrInst *BaseAnalyzer::getFreeStructEleInfo(Instruction *val) {
  if (auto GEle = dyn_cast<GetElementPtrInst>(val)) return GEle;

  LoadInst *l_inst = find_load(val);
  if (l_inst != NULL && l_inst->getOperandList() != NULL) {
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

bool BaseAnalyzer::isStructFree(Instruction *val) {
  // if (auto BCI = dyn_cast<BitCastInst>(val)) {
  //     if(get_type(BCI->getSrcTy())->isStructTy())
  //         return true;
  // }
  if (getStructFreedValue(val) != NULL) return true;
  return false;
}

bool BaseAnalyzer::isOptimizedStructFree(Instruction *I) {
  return getFunctionInformation()->aliasedTypeExists(I);
}

Type *BaseAnalyzer::getOptimizedStructFree(Instruction *I) {
  return getFunctionInformation()->getAliasedType(I);
}

Type *BaseAnalyzer::getStructType(Instruction *val) {
  LoadInst *load_inst = find_load(val);
  if (load_inst && load_inst->getOperandList() != NULL) {
    Type *tgt_type = get_type(load_inst->getPointerOperandType());
    if (tgt_type && get_type(tgt_type)->isStructTy()) return tgt_type;
  }
  return NULL;
}

bool BaseAnalyzer::isFuncPointer(Type *t) {
  Type *tgt = get_type(t);
  if (tgt->isFunctionTy()) return true;
  return false;
}

Value *BaseAnalyzer::getStructFreedValue(Instruction *val,
                                         bool isUserDefCalled) {
  LoadInst *load_inst = find_load(val);
  if (load_inst && load_inst->getOperandList() != NULL) {
    Type *tgt_type = get_type(load_inst->getPointerOperandType());
    if (tgt_type)
      if (isa<StructType>(get_type(tgt_type))) {
        return getLoadeeValue(load_inst);
      }
  } else if (isUserDefCalled) {
    Value *V = val;
    if (auto *BCI = dyn_cast<BitCastInst>(val)) V = BCI->getOperand(0);

    if (this->getFunctionInformation()->aliasedTypeExists(V))
      if (isa<StructType>(
              get_type(this->getFunctionInformation()->getAliasedType(V))))
        return V;
  }
  return NULL;
}

Value *BaseAnalyzer::getCalledStructFreedValue(Instruction *val) {
  return this->getStructFreedValue(val, true);
}

Value *BaseAnalyzer::getFreedValue(Instruction *val) {
  LoadInst *load_inst = find_load(val);
  if (load_inst != NULL && load_inst->getOperandList() != NULL)
    return getLoadeeValue(load_inst);
  return NULL;
}

GetElementPtrInst *BaseAnalyzer::getStoredStructEle(StoreInst *SI) {
  if (auto LInst = dyn_cast<LoadInst>(SI->getValueOperand()))
    if (auto GEle = dyn_cast<GetElementPtrInst>(LInst->getPointerOperand()))
      return GEle;
  return NULL;
}

GetElementPtrInst *BaseAnalyzer::getStoredStruct(StoreInst *SI) {
  Value *v = SI->getPointerOperand();
  if (auto BCI = dyn_cast<CastInst>(v)) v = BCI->getOperand(0);
  if (GetElementPtrInst *GEle = dyn_cast<GetElementPtrInst>(v)) return GEle;
  return NULL;
}

vector<pair<Type *, long>> BaseAnalyzer::decodeGEPInst(
    GetElementPtrInst *GEle) {
  Type *Ty = GEle->getSourceElementType();
  vector<long> indice = getValueIndices(GEle);
  vector<pair<Type *, long>> decoded;

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
          index =
              getMemberIndiceFromByte(cast<StructType>(get_type(Ty)), index);
        }
      }
      decoded.push_back(pair<Type *, long>(Ty, index));
      if (auto StTy = dyn_cast<StructType>(Ty))
        Ty = StTy->getElementType(index);
    }
  }

  return decoded;
}

Type *BaseAnalyzer::extractResultElementType(GetElementPtrInst *GEle) {
  Type *Ty = GEle->getResultElementType();

  if (get_type(Ty)->isIntegerTy()) {
    for (User *usr : GEle->users()) {
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
      if (!stManage->structHoldsAuthority(StTy, ele.second)) return false;
    }
  }
  return true;
}

bool BaseAnalyzer::isAllocCast(CastInst *cast) {
  if (auto CI = dyn_cast<CallInst>(cast->getOperand(0)))
    if (isAllocFunction(CI->getCalledFunction())) return true;
  return false;
}

bool BaseAnalyzer::isCastToVoid(CastInst *CI) {
  // TODO: need more fine-grained checks
  if (auto BCI = dyn_cast<BitCastInst>(CI)) {
    if (get_type(BCI->getDestTy())->isIntegerTy()) {
      return true;
    }
  } else if (isa<PtrToIntInst>(CI)) {
    return true;
  }
  return false;
}

void BaseAnalyzer::collectStructMemberFreeInfo(
    Instruction *I, struct BaseAnalyzer::collectedInfo &info,
    ParentList &additionalParents) {
  GetElementPtrInst *GEle = getFreeStructEleInfo(I);
  if (GEle != NULL) {
    this->getStructParents(GEle, info.indexes);
    GetElementPtrInst *tmpGEle = GEle;
    if (isa<GetElementPtrInst>(GEle->getPointerOperand()))
      tmpGEle = getRootGEle(GEle);
    UpdateIfNull(info.freeValue, getLoadeeValue(tmpGEle->getPointerOperand()));
  }

  for (auto addParent : additionalParents) info.indexes.push_back(addParent);

  if (info.indexes.size() > 0) {
    info.index = info.indexes.back().second;

    if (auto StTy = dyn_cast<StructType>(get_type(info.indexes.back().first))) {
      if (0 <= info.index && info.index < StTy->getNumElements())
        UpdateIfNull(info.memType, StTy->getElementType(info.index));
      else if (ROOT_INDEX < info.index) {
        // TODO: add solid support to negative indice of GEP (a.k.a.
        // container_of)
      }
    }

    if (get_type(info.indexes.front().first)->isStructTy())
      UpdateIfNull(info.parentType,
                   cast<StructType>(get_type(info.indexes.front().first)));

    UpdateIfNull(info.freeValue, getCalledStructFreedValue(I));
    info.isStructRelated = true;
    generateWarning(I, "Struct element free collected", true);
  }
  return;
}

void BaseAnalyzer::collectSimpleFreeInfo(
    Instruction *I, struct BaseAnalyzer::collectedInfo &info) {
  UpdateIfNull(info.freeValue, getFreedValue(I));
  if (info.freeValue) UpdateIfNull(info.memType, info.freeValue->getType());
  generateWarning(I, "Value Free");
  return;
}

void BaseAnalyzer::collectStructFreeInfo(
    Instruction *I, struct BaseAnalyzer::collectedInfo &info) {
  Value *loaded_value = getStructFreedValue(I);
  if (loaded_value) {
    UpdateIfNull(info.freeValue, loaded_value);
    UpdateIfNull(info.memType, getStructType(I));
  }
  info.isStructRelated = true;
  return;
}

void BaseAnalyzer::collectOptimizedStructFreeInfo(
    Instruction *I, struct BaseAnalyzer::collectedInfo &info) {
  UpdateIfNull(info.freeValue, I);
  UpdateIfNull(info.memType, getOptimizedStructFree(I));
  info.isStructRelated = true;
  return;
}

void BaseAnalyzer::addNestedFree(Value *V, CallInst *CI, BasicBlock *B,
                                 struct collectedInfo &info,
                                 ParentList &additionalParents) {
  StructType *StTy = cast<StructType>(get_type(info.memType));
  int memIndex = 0;

  for (auto ele : StTy->elements()) {
    if (ele->isStructTy()) {
      additionalParents.push_back(pair<Type *, int>(ele, memIndex++));
      this->addFree(V, CI, B, false, additionalParents);
      additionalParents.pop_back();
    }
  }
  return;
}

ICmpInst *BaseAnalyzer::findAllocICmp(Instruction *I) {
  ICmpInst *icmp = NULL;

  for (auto usr : I->users()) {
    if (auto ip = dyn_cast<ICmpInst>(usr)) {
      icmp = ip;
      break;
    } else if (auto BCI = dyn_cast<BitCastInst>(usr)) {
      icmp = findAllocICmp(BCI);
    } else if (auto SI = dyn_cast<StoreInst>(usr)) {
      for (auto si_usr : SI->getPointerOperand()->users()) {
        if (auto tmpI = dyn_cast<Instruction>(si_usr)) {
          icmp = findAllocICmp(tmpI);
        }
      }
    }
  }
  return icmp;
}

void BaseAnalyzer::analyzeErrorCode(BranchInst *BI, ICmpInst *ICI,
                                    BasicBlock &B) {
  int op = this->getErrorOperand(ICI);
  CmpInst::Predicate pred = ICI->getPredicate();

  // TODO: update to adjust errocode information
  int errcode = 0;

  if (op >= 0) {
    generateWarning(BI, "Analyzing Error Code", true);
    if (this->errorCodeExists(ICI, B, errcode)) {
      BasicBlock *errBlock = BI->getSuccessor(op);
      this->getFunctionInformation()
          ->getBasicBlockInformation(&B)
          ->addSucceedingErrorBlock(errBlock);

      BasicBlockWorkList allocated_on_err =
          this->getErrorValues(ICI, B, errcode);
      generateWarning(
          BI, "Allocated Err: " + to_string(allocated_on_err.getList().size()),
          true);

      // 1. Get Allocated on Success
      BasicBlockWorkList allocated_on_success = this->getSuccessValues(ICI, B);
      generateWarning(BI,
                      "Allocated Success: " +
                          to_string(allocated_on_success.getList().size()),
                      true);

      // 2. Success diff allocated_on_err is pure non allocated in err
      BasicBlockList diff_list = BasicBlockListOperation::diffList(
          allocated_on_success.getList(), allocated_on_err.getList());

      // 3. add it to the list
      for (auto ele : diff_list) {
        this->getFunctionInformation()
            ->getBasicBlockInformation(&B)
            ->addRemoveAlloc(errBlock, const_cast<UniqueKey *>(ele));
      }
    }
  }
}

void BaseAnalyzer::analyzeNullCheck(BranchInst *BI, ICmpInst *ICI,
                                    BasicBlock &B) {
  generateWarning(BI, "Analyze NULL Check", true);
  BasicBlockWorkList BList;

  // Get which basicblock to pass the data to
  int op = this->getErrorOperand(ICI);
  BasicBlock *errBlock = BI->getSuccessor(op);

  // decode compared type
  Value *comVal = this->getComparedValue(ICI);
  Type *Ty = this->getComparedType(comVal, B);

  // check if the value is struct or not
  ParentList plist = this->decodeErrorTypes(ICI->getOperand(0));
  if (plist.size() > 0) {
    if (auto StTy = dyn_cast<StructType>(get_type(plist.back().first))) {
      if (0 <= plist.back().second &&
          plist.back().second < StTy->getNumElements())
        Ty = StTy->getElementType(plist.back().second);
    }
    BList.add(
        this->getFunctionInformation()->getUniqueKeyManager()->getUniqueKey(
            NULL, Ty, plist.back().second));
  }

  if (this->getFunctionInformation()->isAllocatedInBasicBlock(&B, NULL, Ty,
                                                              ROOT_INDEX)) {
    BList.add(
        this->getFunctionInformation()->getUniqueKeyManager()->getUniqueKey(
            NULL, Ty, ROOT_INDEX));
  }

  generateWarning(BI, "Adding Null value", true);
  for (auto ele : BList.getList()) {
    this->getFunctionInformation()
        ->getBasicBlockInformation(&B)
        ->addRemoveAlloc(errBlock, const_cast<UniqueKey *>(ele));
  }
}

void BaseAnalyzer::analyzeErrorCheckFunction(BranchInst *BI, CallInst *CI,
                                             BasicBlock &B) {
  generateWarning(CI, "Calling IS_ERR()");
  BasicBlock *errBlock = BI->getSuccessor(0);
  this->getFunctionInformation()
      ->getBasicBlockInformation(&B)
      ->addSucceedingErrorBlock(errBlock);

  Value *tgt_val = CI->getArgOperand(0);
  if (auto BCI = dyn_cast<BitCastInst>(tgt_val)) {
    tgt_val = BCI->getOperand(0);
  }
  ParentList plist = this->decodeErrorTypes(tgt_val);
  Type *Ty = this->getComparedType(decodeComparedValue(tgt_val), B);
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
          ->addRemoveAlloc(
              errBlock, const_cast<UniqueKey *>(
                            this->getFunctionInformation()
                                ->getUniqueKeyManager()
                                ->getUniqueKey(NULL, Ty, plist.back().second)));
    }
  }
  if (this->getFunctionInformation()->isAllocatedInBasicBlock(errBlock, NULL,
                                                              Ty, ROOT_INDEX)) {
    this->getFunctionInformation()
        ->getBasicBlockInformation(&B)
        ->addRemoveAlloc(
            errBlock,
            const_cast<UniqueKey *>(this->getFunctionInformation()
                                        ->getUniqueKeyManager()
                                        ->getUniqueKey(NULL, Ty, ROOT_INDEX)));
  }
}

BasicBlockWorkList BaseAnalyzer::getErrorValues(Instruction *I, BasicBlock &B,
                                                int errcode) {
  BasicBlockWorkList BList;
  ICmpInst *ICI = cast<ICmpInst>(I);
  Value *comVal = this->getComparedValue(ICI);

  if (isa<ConstantInt>(ICI->getOperand(1))) {
    CallInst *CI = NULL;
    generateWarning(I, "Compare with Int: Error Code");
    if (auto comValCI = dyn_cast<CallInst>(comVal)) {
      CI = comValCI;
    } else {
      CI = this->getFunctionInformation()
               ->getBasicBlockInformation(&B)
               ->getCallInstForVal(comVal);
    }
    if (CI) {
      generateWarning(CI, "Error Code");
      for (auto ele : this->getErrorAllocInCalledFunction(CI, errcode)) {
        BList.add(ele);
      }
    }
  } else if (isa<ConstantPointerNull>(ICI->getOperand(1))) {
    CallInst *CI = NULL;
    generateWarning(I, "Compare with NULL: Error Code");
    if (auto comValCI = dyn_cast<CallInst>(comVal)) {
      CI = comValCI;
    } else {
      CI = this->getFunctionInformation()
               ->getBasicBlockInformation(&B)
               ->getCallInstForVal(comVal);
    }
    if (CI) {
      generateWarning(CI, "Error Code");
      for (auto ele : this->getErrorAllocInCalledFunction(CI, 0)) {
        BList.add(ele);
      }
    }
    // generateWarning(I, "Compare with NULL: Look at allocation", true);
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

BasicBlockWorkList BaseAnalyzer::getSuccessValues(Instruction *I,
                                                  BasicBlock &B) {
  BasicBlockWorkList BList;
  ICmpInst *ICI = cast<ICmpInst>(I);
  Value *comVal = this->getComparedValue(ICI);

  if (isa<ConstantInt>(ICI->getOperand(1))) {
    CallInst *CI = NULL;
    generateWarning(I, "Compare with Int: Error Code");
    if (auto comValCI = dyn_cast<CallInst>(comVal)) {
      CI = comValCI;
    } else {
      CI = this->getFunctionInformation()
               ->getBasicBlockInformation(&B)
               ->getCallInstForVal(comVal);
    }
    if (CI) {
      generateWarning(CI, "Error Code");
      for (auto ele : this->getSuccessAllocInCalledFunction(CI)) {
        BList.add(ele);
      }
    }
  } else if (isa<ConstantPointerNull>(ICI->getOperand(1))) {
    CallInst *CI = NULL;
    generateWarning(I, "Compare with Int: Error Code");
    if (auto comValCI = dyn_cast<CallInst>(comVal)) {
      CI = comValCI;
    } else {
      CI = this->getFunctionInformation()
               ->getBasicBlockInformation(&B)
               ->getCallInstForVal(comVal);
    }
    if (CI) {
      generateWarning(CI, "Error Code");
      for (auto ele : this->getSuccessAllocInCalledFunction(CI)) {
        BList.add(ele);
      }
    }
  }
  return BList;
}

bool BaseAnalyzer::errorCodeExists(Instruction *I, BasicBlock &B, int errcode) {
  ICmpInst *ICI = cast<ICmpInst>(I);
  Value *comVal = this->getComparedValue(ICI);

  if (isa<ConstantInt>(ICI->getOperand(1))) {
    CallInst *CI = NULL;
    generateWarning(I, "Compare with Int: Error Code", true);
    if (auto comValCI = dyn_cast<CallInst>(comVal)) {
      CI = comValCI;
    } else {
      CI = this->getFunctionInformation()
               ->getBasicBlockInformation(&B)
               ->getCallInstForVal(comVal);
    }
    if (CI) {
      generateWarning(CI, "Error Code");
      Function *DF = CI->getCalledFunction();
      return this->getFunctionManager()
          ->getElement(DF)
          ->errorCodeLessThanExists(errcode);
    }
  } else if (isa<ConstantPointerNull>(ICI->getOperand(1))) {
    return true;
  }
  return false;
}

Value *BaseAnalyzer::getComparedValue(ICmpInst *ICI) {
  return this->decodeComparedValue(ICI->getOperand(0));
}

Value *BaseAnalyzer::decodeComparedValue(Value *V) {
  Value *comVal = V;
  if (this->getFunctionInformation()->aliasExists(comVal)) {
    Value *aliased_value = this->getFunctionInformation()->getAlias(comVal);
    if (auto GEle = dyn_cast<GetElementPtrInst>(aliased_value)) {
      comVal = aliased_value;
    }
  }

  if (auto LI = dyn_cast<LoadInst>(comVal)) {
    comVal = LI->getPointerOperand();
  }

  if (auto GEle = dyn_cast<GetElementPtrInst>(comVal)) {
    GEle = getRootGEle(GEle);
    comVal = getLoadeeValue(GEle->getPointerOperand());
  }
  return comVal;
}

ParentList BaseAnalyzer::decodeErrorTypes(Value *V) {
  ParentList plist;
  Value *comVal = V;
  if (this->getFunctionInformation()->aliasExists(comVal)) {
    Value *aliased_value = this->getFunctionInformation()->getAlias(comVal);
    if (auto GEle = dyn_cast<GetElementPtrInst>(aliased_value)) {
      comVal = aliased_value;
    }
  }
  if (auto LI = dyn_cast<LoadInst>(comVal)) {
    comVal = LI->getPointerOperand();
  }
  if (auto BCI = dyn_cast<BitCastInst>(comVal)) {
    comVal = BCI->getOperand(0);
  }

  if (auto GEle = dyn_cast<GetElementPtrInst>(comVal)) {
    this->getStructParents(GEle, plist);
  }
  return plist;
}

Type *BaseAnalyzer::getComparedType(Value *comVal, BasicBlock &B) {
  Type *Ty = comVal->getType();
  if (this->getFunctionInformation()
          ->getBasicBlockInformation(&B)
          ->isCallValues(comVal)) {
    if (auto Alloca = dyn_cast<AllocaInst>(comVal)) {
      Ty = Alloca->getAllocatedType();
    }
  }

  if (auto CI = dyn_cast<CallInst>(comVal)) {
    Ty = CI->getType();
    for (auto usr : CI->users()) {
      if (auto CastI = dyn_cast<CastInst>(usr)) Ty = CastI->getDestTy();
    }
  }
  return Ty;
}

int BaseAnalyzer::getErrorOperand(ICmpInst *ICI) {
  int operand = -1;
  if (auto ConstI = dyn_cast<ConstantInt>(ICI->getOperand(1))) {
    if (ConstI->isZero()) {
      // TODO: check for each case
      if (ICI->getPredicate() == CmpInst::ICMP_EQ ||
          ICI->getPredicate() == CmpInst::ICMP_SGE)
        operand = 1;
      else if (ICI->getPredicate() == CmpInst::ICMP_NE ||
               ICI->getPredicate() == CmpInst::ICMP_SLT)
        operand = 0;
    }
  } else if (isa<ConstantPointerNull>(ICI->getOperand(1))) {
    if (ICI->getPredicate() == CmpInst::ICMP_EQ)
      operand = 0;
    else if (ICI->getPredicate() == CmpInst::ICMP_NE)
      operand = 1;
  }
  return operand;
}

BasicBlockList BaseAnalyzer::getErrorAllocInCalledFunction(CallInst *CI,
                                                           int errcode) {
  Function *DF = CI->getCalledFunction();
  generateWarning(CI, "Called Error");
  return this->getFunctionManager()->getElement(DF)->getAllocatedInError(
      errcode);
}

BasicBlockList BaseAnalyzer::getSuccessAllocInCalledFunction(CallInst *CI) {
  Function *DF = CI->getCalledFunction();
  generateWarning(CI, "Called Success");
  return this->getFunctionManager()->getElement(DF)->getAllocatedInSuccess();
}

void BaseAnalyzer::buildReturnValueInformation() {
  ReturnInst *RI = getFunctionInformation()->getReturnInst();
  BasicBlock *B = getFunctionInformation()->getEndPoint();

  Type *RetTy = getFunctionInformation()->getFunction().getReturnType();
  if (RetTy->isIntegerTy()) {
    if (RI->getNumOperands() <= 0) return;
    Value *V = RI->getReturnValue();
    this->checkErrorInstruction(V);
  } else if (RetTy->isPointerTy()) {
    // TODO: add support to pointers
    generateWarning(RI, "[RETURN]: No Error Code Analysis");
    getFunctionInformation()->addSuccessBlockInformation(B);
  } else {
    generateWarning(RI, "[RETURN]: No Error Code Analysis");
    getFunctionInformation()->addSuccessBlockInformation(B);
  }
}

void BaseAnalyzer::checkErrorCodeAndAddBlock(
    Instruction *I, BasicBlock *B, Value *inval,
    vector<Instruction *> visited_inst) {
  if (auto CInt = dyn_cast<ConstantInt>(inval)) {
    generateWarning(I, "Storing constant value to ret");
    int64_t errcode = CInt->getSExtValue();
    if (errcode < NO_ERROR) {
      generateWarning(I, "[RETURN] ERR: " + to_string(errcode), true);
      getFunctionInformation()->addErrorBlockInformation(errcode, B);
    } else {
      generateWarning(I, "[RETURN] SUCCESS: " + to_string(errcode), true);
      getFunctionInformation()->addSuccessBlockInformation(B);
    }
  } else if (auto CI = dyn_cast<CallInst>(inval)) {
    generateWarning(I, "Storing caall inst value to ret", true);
    if (FunctionInformation *DF =
            getFunctionManager()->getElement(CI->getCalledFunction())) {
      for (auto err_code_info : DF->getErrorCodeMap()) {
        int64_t errcode = err_code_info.first;

        if (errcode < NO_ERROR) {
          generateWarning(I, "[RETURN][CALLINST] ERR: " + to_string(errcode),
                          true);
          getFunctionInformation()->addErrorBlockFreeInformation(
              errcode, err_code_info.second.free_list);
          getFunctionInformation()->addErrorBlockAllocInformation(
              errcode, err_code_info.second.alloc_list);
        } else {
          generateWarning(
              I, "[RETURN][CALLINST] SUCCESS: " + to_string(errcode), true);
          getFunctionInformation()->addErrorBlockFreeInformation(
              0, err_code_info.second.free_list);
          getFunctionInformation()->addErrorBlockAllocInformation(
              0, err_code_info.second.alloc_list);
        }
      }
    }
  } else if (inval->getType()->isPointerTy()) {
    if (isa<ConstantPointerNull>(inval)) {
      generateWarning(I, "[RETURN] ERR: NULL", true);
      getFunctionInformation()->addErrorBlockInformation(-1, B);
    } else {
      generateWarning(I, "[RETURN] SUCCESS: Non-NULL", true);
      getFunctionInformation()->addSuccessBlockInformation(B);
    }
  } else {
    if (auto PHI = dyn_cast<PHINode>(inval)) {
      generateWarning(PHI, "[ERRORINST]: PHINode Instruction Reivisted", true);
      if (find(visited_inst.begin(), visited_inst.end(), PHI) ==
          visited_inst.end()) {
        visited_inst.push_back(PHI);
        checkErrorInstruction(PHI, visited_inst);
      }
    }
  }
  return;
}

void BaseAnalyzer::checkErrorInstruction(Value *V,
                                         vector<Instruction *> visited_inst) {
  if (auto CInt = dyn_cast<Constant>(V)) {
    // generateWarning(RI, "Const Int");
  }
  if (auto CI = dyn_cast<CallInst>(V)) {
    generateWarning(CI, "[ERRORINST]: Call Inst", true);
    if (CI->getCalledFunction()) {
      generateWarning(CI, CI->getFunction()->getName(), true);
    }
  }
  if (auto LI = dyn_cast<LoadInst>(V)) {
    generateWarning(LI, "[ERRORINST]: Load Instruction", true);
    for (auto usr : LI->getPointerOperand()->users()) {
      if (auto SI = dyn_cast<StoreInst>(usr)) {
        if (V != SI->getValueOperand())
          this->checkErrorCodeAndAddBlock(SI, SI->getParent(),
                                          SI->getValueOperand(), visited_inst);
      }
    }
  } else if (auto PHI = dyn_cast<PHINode>(V)) {
    generateWarning(PHI, "[ERRORINST]: PHINode Instruction", true);
    for (unsigned i = 0; i < PHI->getNumIncomingValues(); i++) {
      if (V != PHI->getIncomingValue(i))
        this->checkErrorCodeAndAddBlock(PHI, PHI->getIncomingBlock(i),
                                        PHI->getIncomingValue(i), visited_inst);
    }
  }
  return;
}

bool BaseAnalyzer::isBidirectionalAlias(Value *V) {
  if (Value *aliasVal = getFunctionInformation()->getAlias(V)) {
    if (Instruction *I = dyn_cast<Instruction>(aliasVal)) {
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
    BasicBlock *B) {
  for (BasicBlock *preds : predecessors(B)) {
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

bool BaseAnalyzer::isCallInstReturnValue(Value *V) {
  Value *tgt_val = V;
  if (auto LI = dyn_cast<LoadInst>(tgt_val)) {
    tgt_val = LI->getPointerOperand();
  }

  if (auto CI = dyn_cast<CallInst>(tgt_val)) return true;

  return false;
}
}  // namespace ST_free
