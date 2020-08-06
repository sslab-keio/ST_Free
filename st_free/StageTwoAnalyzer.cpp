#include "include/StageTwoAnalyzer.hpp"

namespace ST_free {

void StageTwoAnalyzer::analyzeInstructions(BasicBlock &B) {
  for (llvm::Instruction &I : B) {
    if (this->isReturnFunc(&I)) getFunctionInformation()->addEndPoint(&B);

    if (llvm::AllocaInst *AI = llvm::dyn_cast<llvm::AllocaInst>(&I)) {
      this->analyzeAllocaInst(AI, B);
    } else if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(&I)) {
      this->analyzeCallInst(CI, B);
    } else if (llvm::StoreInst *SI = llvm::dyn_cast<llvm::StoreInst>(&I)) {
      this->analyzeStoreInst(SI, B);
    } else if (llvm::BranchInst *BI = llvm::dyn_cast<llvm::BranchInst>(&I)) {
      this->analyzeBranchInst(BI, B);
    }
  }
}

void StageTwoAnalyzer::analyzeAllocaInst(llvm::AllocaInst *AI, llvm::BasicBlock &B) {
  // this->addLocalVariable(
  //         &B,
  //         ainst->getAllocatedType(),
  //         ainst,
  //         cast<Instruction>(getFirstUser(&I)),
  //         ParentList()
  //     );
}

void StageTwoAnalyzer::analyzeStoreInst(llvm::StoreInst *SI, llvm::BasicBlock &B) {
  // if(this->isStoreToStruct(SI)){
  // }
  if (this->isStoreToStructMember(SI)) {
    generateWarning(SI, "is Store to struct");
    llvm::GetElementPtrInst *GEle = getStoredStruct(SI);
    getStructManager()->addStore(llvm::cast<llvm::StructType>(GEle->getSourceElementType()),
                                 getValueIndices(GEle));

    if (llvm::isa<llvm::GlobalValue>(SI->getValueOperand())) {
      generateWarning(SI, "GolbalVariable Store");
      getStructManager()->addGlobalVarStore(
          llvm::cast<llvm::StructType>(GEle->getSourceElementType()),
          getValueIndices(GEle));
    }
    // if(isa<AllocaInst>(SI->getValueOperand())){
    //     getFunctionInformation()->setAliasInBasicBlock(&B, GEle,
    //     SI->getValueOperand());
    // }
  }

  if (this->isStoreFromStructMember(SI)) {
    generateWarning(SI, "is Store from struct");
    llvm::GetElementPtrInst *GEle = getStoredStructEle(SI);
    if (llvm::isa<llvm::AllocaInst>(SI->getPointerOperand())) {
      getFunctionInformation()->setAliasInBasicBlock(&B, GEle,
                                                     SI->getPointerOperand());
    }
  }
}

void StageTwoAnalyzer::analyzeCallInst(llvm::CallInst *CI, llvm::BasicBlock &B) {
  if (llvm::Function *called_function = CI->getCalledFunction()) {
    if (isAllocFunction(called_function)) {
      llvm::Value *val = getAllocatedValue(CI);
      if (val != NULL)
        if (llvm::StructType *strTy =
                llvm::dyn_cast<llvm::StructType>(get_type(val->getType()))) {
          getStructManager()->addAlloc(strTy);
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

void StageTwoAnalyzer::analyzeBranchInst(llvm::BranchInst *BI, llvm::BasicBlock &B) {
  if (this->isCorrectlyBranched(BI)) {
    generateWarning(BI, "Correctly Branched");
    getFunctionInformation()->setCorrectlyBranched(&B);
  }
}

void StageTwoAnalyzer::checkAvailability() {
  FreedStructList fsl = getFunctionInformation()->getFreedStruct();

  for (FreedStruct *freedStruct : fsl) {
    llvm::StructType *strTy = llvm::cast<llvm::StructType>(freedStruct->getType());
    int cPointers = strTy->getNumElements();
    vector<bool> alreadyFreed = freedStruct->getFreedMember();
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

void StageTwoAnalyzer::analyzeDifferentFunc(llvm::Function &F) {
  StageTwoAnalyzer called_function(&F, getStructManager());
  called_function.analyze();
  return;
}
}  // namespace ST_free
