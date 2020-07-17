#include "include/StageTwoAnalyzer.hpp"

namespace ST_free {

void StageTwoAnalyzer::analyzeInstructions(BasicBlock &B) {
  for (Instruction &I : B) {
    if (this->isReturnFunc(&I)) getFunctionInformation()->addEndPoint(&B);

    if (AllocaInst *AI = dyn_cast<AllocaInst>(&I)) {
      this->analyzeAllocaInst(AI, B);
    } else if (CallInst *CI = dyn_cast<CallInst>(&I)) {
      this->analyzeCallInst(CI, B);
    } else if (StoreInst *SI = dyn_cast<StoreInst>(&I)) {
      this->analyzeStoreInst(SI, B);
    } else if (BranchInst *BI = dyn_cast<BranchInst>(&I)) {
      this->analyzeBranchInst(BI, B);
    }
  }
}

void StageTwoAnalyzer::analyzeAllocaInst(AllocaInst *AI, BasicBlock &B) {
  // this->addLocalVariable(
  //         &B,
  //         ainst->getAllocatedType(),
  //         ainst,
  //         cast<Instruction>(getFirstUser(&I)),
  //         ParentList()
  //     );
}

void StageTwoAnalyzer::analyzeStoreInst(StoreInst *SI, BasicBlock &B) {
  // if(this->isStoreToStruct(SI)){
  // }
  if (this->isStoreToStructMember(SI)) {
    generateWarning(SI, "is Store to struct");
    GetElementPtrInst *GEle = getStoredStruct(SI);
    getStructManager()->addStore(cast<StructType>(GEle->getSourceElementType()),
                                 getValueIndices(GEle));

    if (isa<GlobalValue>(SI->getValueOperand())) {
      generateWarning(SI, "GolbalVariable Store");
      getStructManager()->addGlobalVarStore(
          cast<StructType>(GEle->getSourceElementType()),
          getValueIndices(GEle));
    }
    // if(isa<AllocaInst>(SI->getValueOperand())){
    //     getFunctionInformation()->setAliasInBasicBlock(&B, GEle,
    //     SI->getValueOperand());
    // }
  }

  if (this->isStoreFromStructMember(SI)) {
    generateWarning(SI, "is Store from struct");
    GetElementPtrInst *GEle = getStoredStructEle(SI);
    if (isa<AllocaInst>(SI->getPointerOperand())) {
      getFunctionInformation()->setAliasInBasicBlock(&B, GEle,
                                                     SI->getPointerOperand());
    }
  }
}

void StageTwoAnalyzer::analyzeCallInst(CallInst *CI, BasicBlock &B) {
  if (Function *called_function = CI->getCalledFunction()) {
    if (isAllocFunction(called_function)) {
      Value *val = getAllocatedValue(CI);
      if (val != NULL)
        if (StructType *strTy =
                dyn_cast<StructType>(get_type(val->getType()))) {
          getStructManager()->addAlloc(strTy);
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

void StageTwoAnalyzer::analyzeBranchInst(BranchInst *BI, BasicBlock &B) {
  if (this->isCorrectlyBranched(BI)) {
    generateWarning(BI, "Correctly Branched");
    getFunctionInformation()->setCorrectlyBranched(&B);
  }
}

void StageTwoAnalyzer::checkAvailability() {
  FreedStructList fsl = getFunctionInformation()->getFreedStruct();

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
        }

        if (isFreed) {
          getFunctionInformation()->setStructMemberFreed(freedStruct,
                                                         vinfo->getMemberNum());
          if (getFunctionInformation()->isArgValue(vinfo->getValue())) {
            getFunctionInformation()->setStructMemberArgFreed(
                vinfo->getValue(), vinfo->getMemberNum());
          }
        }
      }
    }

    if (!getFunctionInformation()->isArgValue(freedStruct->getValue())) {
      getStructManager()->addCandidateValue(
          &(getFunctionInformation()->getFunction()), strTy, freedStruct);
    }
  }
  return;
}

void StageTwoAnalyzer::analyzeDifferentFunc(Function &F) {
  StageTwoAnalyzer called_function(&F, getStructManager());
  called_function.analyze();
  return;
}
}  // namespace ST_free
