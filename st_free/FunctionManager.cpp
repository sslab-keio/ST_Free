#include "include/FunctionManager.hpp"

namespace ST_free {
UniqueKeyManager FunctionInformation::UKManage;

bool FunctionManager::exists(Function *F) {
  if (func_map.find(F) != func_map.end()) return true;
  return false;
}
FunctionInformation *FunctionManager::getElement(Function *F) {
  if (!this->exists(F)) func_map[F] = new FunctionInformation(F);
  return func_map[F];
}

FunctionInformation::FunctionInformation(Function *Func) {
  if (Func != NULL) {
    F = Func;
    args = ArgList(Func->arg_size());
    args.setArgs(Func);
    stat = UNANALYZED;
  }
}

FunctionInformation::FunctionInformation() { stat = UNANALYZED; }

Function &FunctionInformation::getFunction() { return (Function &)(*this->F); }

int FunctionInformation::getStat() { return this->stat; }

void FunctionInformation::setStat(int stat) { this->stat = stat; }

void FunctionInformation::addEndPoint(BasicBlock *B) { endPoint.push_back(B); }

void FunctionInformation::addSuccessBlock(BasicBlock *B) {
  successBlock.push_back(B);
}

void FunctionInformation::addErrorBlock(int64_t errcode, BasicBlock *B) {
  errorBlock.push_back(pair<int64_t, BasicBlock *>(errcode, B));
}

ValueInformation *FunctionInformation::addFreeValue(BasicBlock *B, Value *V,
                                                    Type *memTy, Type *stTy,
                                                    long num,
                                                    ParentList plist) {
  const UniqueKey *UK =
      this->getUniqueKeyManager()->getUniqueKey(V, memTy, num);
  if (UK == NULL) UK = this->getUniqueKeyManager()->addUniqueKey(V, memTy, num);

  ValueInformation *varinfo = this->getValueInfo(UK);
  if (varinfo == NULL)
    varinfo = this->addVariable(UK, V, memTy, stTy, num, plist);
  else
    varinfo->addStructParams(stTy, num);
  varinfo->setFreed();

  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) {
    BInfo->addFree(UK);
    if (BBManage.isPredBlockCorrectlyBranched(B) || BInfo->isLoopBlock()) {
      this->addCorrectlyFreedValue(B, UK);
    }
  }
  return varinfo;
}

void FunctionInformation::addFreeValueFromDifferentFunction(
    BasicBlock *B, ValueInformation *VI) {
  if (VI != NULL)
    this->addFreeValue(B, VI->getValue(), VI->getMemberType(),
                       VI->getStructType(), VI->getMemberNum(),
                       VI->getParents());
  return;
}

void FunctionInformation::addFreeValue(BasicBlock *B, UniqueKey *UK) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) BInfo->addFree(UK);
}

void FunctionInformation::addAllocValue(BasicBlock *B, Value *V, Type *T,
                                        long mem) {
  const UniqueKey *UK = this->getUniqueKeyManager()->getUniqueKey(V, T, mem);
  if (UK == NULL) UK = this->getUniqueKeyManager()->addUniqueKey(V, T, mem);

  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) BInfo->addAlloc(UK);
}

void FunctionInformation::addAllocValue(BasicBlock *B, UniqueKey *UK) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) BInfo->addAlloc(UK);
}

void FunctionInformation::addPendingArgAlloc(BasicBlock *B, Value *V, Type *T,
                                             long mem) {
  const UniqueKey *UK = this->getUniqueKeyManager()->getUniqueKey(V, T, mem);
  if (UK == NULL) UK = this->getUniqueKeyManager()->addUniqueKey(V, T, mem);

  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) BInfo->addPendingArgAlloc(UK);
}

void FunctionInformation::addPendingArgAlloc(BasicBlock *B, UniqueKey *UK) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) BInfo->addPendingArgAlloc(UK);
}

bool FunctionInformation::isUnanalyzed() {
  return getStat() == UNANALYZED ? true : false;
}

bool FunctionInformation::isInProgress() {
  return getStat() == IN_PROGRESS ? true : false;
}

bool FunctionInformation::isAnalyzed() {
  return getStat() == ANALYZED ? true : false;
}

void FunctionInformation::setAnalyzed() { setStat(ANALYZED); }

void FunctionInformation::setInProgress() { setStat(IN_PROGRESS); }

void FunctionInformation::BBCollectInfo(BasicBlock &B, bool isEntryPoint) {
  BBManage.CollectInInfo(&B, isEntryPoint);
}

void FunctionInformation::addFreedStruct(Type *T, Value *V, Instruction *I) {
  freedStruct.push_back(new FreedStruct(T, V, I));
}

void FunctionInformation::addFreedStruct(BasicBlock *B, Type *T, Value *V,
                                         Instruction *I) {
  FreedStruct *fst = new FreedStruct(T, V, I, B, NULL);
  if (!this->freedStructExists(fst)) {
    freedStruct.push_back(fst);
  }
}

void FunctionInformation::addFreedStruct(BasicBlock *B, Type *T, Value *V,
                                         Instruction *I, StructType *parent,
                                         ValueInformation *valInfo,
                                         bool isInStruct) {
  FreedStruct *fst = new FreedStruct(T, V, I, B, valInfo, isInStruct);
  if (!this->freedStructExists(fst))
    freedStruct.push_back(fst);
  else
    delete fst;
}

bool FunctionInformation::freedStructExists(FreedStruct *fst) {
  if (find(freedStruct.begin(), freedStruct.end(), fst) != freedStruct.end())
    return true;
  return false;
}

void FunctionInformation::addParentType(Type *T, Value *V, Instruction *I,
                                        StructType *parentTy, int ind) {
  FreedStruct fst(T, V, I);
  auto fVal = find(freedStruct.begin(), freedStruct.end(), &fst);
  if (fVal != freedStruct.end() && parentTy != NULL)
    (*fVal)->addParent(parentTy, ind);
}

vector<BasicBlock *> FunctionInformation::getEndPoint() const {
  return endPoint;
}

vector<BasicBlock *> FunctionInformation::getSuccessBlock() const {
  return successBlock;
}

vector<pair<int64_t, BasicBlock *>> FunctionInformation::getErrorBlock() const {
  return errorBlock;
}

FreedStructList FunctionInformation::getFreedStruct() const {
  return freedStruct;
}

BasicBlockList FunctionInformation::getFreeList(BasicBlock *B) {
  return BBManage.getBasicBlockFreeList(B);
}

BasicBlockList FunctionInformation::getAllocList(BasicBlock *B) {
  return BBManage.getBasicBlockAllocList(B);
}

BasicBlockList FunctionInformation::getPendingStoreList(BasicBlock *B) {
  return BBManage.getBasicBlockPendingAllocList(B);
}

bool FunctionInformation::isArgValue(Value *v) { return args.isInList(v); }

void FunctionInformation::setArgFree(Value *V) { args.setFreed(V); }

void FunctionInformation::setArgAlloc(Value *V) {
  // args.setAllocated(V);
}

void FunctionInformation::setStructArgFree(Value *V, int64_t num) {
  // args.setFreedStructNumber(V, num);
}

void FunctionInformation::setStructArgAlloc(Value *V, int64_t num) {
  // args.setAllocatedStructNumber(V, num);
}

void FunctionInformation::setStructMemberArgFreed(Value *V,
                                                  ParentList indexes) {
  vector<int> ind;
  for (auto i : indexes) {
    if (i.second != ROOT_INDEX) ind.push_back(i.second);
  }
  args.setFreed(V, ind);
}

void FunctionInformation::setStructMemberArgAllocated(Value *V, int64_t num) {
  // args.setStructMemberAllocated(V, num);
}

bool FunctionInformation::isArgFreed(int64_t num) { return false; }

bool FunctionInformation::isArgAllocated(int64_t num) {
  // args.isArgAllocated(num);
}

ValueInformation *FunctionInformation::addVariable(Value *val) {
  const UniqueKey *UK = this->getUniqueKeyManager()->getUniqueKey(
      val, val->getType(), ROOT_INDEX);
  if (UK == NULL)
    UK = this->getUniqueKeyManager()->addUniqueKey(val, val->getType(),
                                                   ROOT_INDEX);
  if (!VManage.exists(UK)) VManage.addValueInfo(UK, val);
  return VManage.getValueInfo(UK);
}

ValueInformation *FunctionInformation::addVariable(const UniqueKey *UK,
                                                   Value *val, Type *memType,
                                                   Type *parType, long num,
                                                   ParentList plist) {
  if (!VManage.exists(UK))
    VManage.addValueInfo(UK, val, memType, parType, num, plist);
  return VManage.getValueInfo(UK);
}

ValueInformation *FunctionInformation::getValueInfo(Value *val, Type *ty,
                                                    long mem) {
  const UniqueKey *UK = this->getUniqueKeyManager()->getUniqueKey(val, ty, mem);
  if (UK != NULL) return this->getValueInfo(UK);
  return NULL;
}

ValueInformation *FunctionInformation::getValueInfo(const UniqueKey *UK) {
  return VManage.getValueInfo(UK);
}

void FunctionInformation::addLocalVar(BasicBlock *B, Type *T, Value *V,
                                      Instruction *I) {
  localVariables.push_back(new FreedStruct(T, V, I));
}

void FunctionInformation::addLocalVar(BasicBlock *B, Type *T, Value *V,
                                      Instruction *I, ParentList P,
                                      ValueInformation *vinfo) {
  localVariables.push_back(new FreedStruct(T, V, I, P, B, vinfo));
}

//     void FunctionInformation::incrementRefCount(Value *V, Type *T, long mem,
//     Value *ref){
//         const UniqueKey *UK = this->getUniqueKeyManager()->getUniqueKey(V, T,
//         mem); if (UK == NULL)
//             UK = this->getUniqueKeyManager()->addUniqueKey(V, T, mem);

//         ValueInformation * vinfo = VManage.getValueInfo(UK);
//         if(vinfo == NULL){
//             VManage.addValueInfo(UK, V, T, mem);
//             vinfo = VManage.getValueInfo(V, T, mem);
//         }
//         vinfo->incrementRefCount(ref);
//     }

// void FunctionInformation::incrementFreedRefCount(BasicBlock *B, Value *V,
// Value *ref){
//     ValueInformation * vinfo = VManage.getValueInfo(V);
//     if(vinfo != NULL)
//         vinfo->incrementRefCount(ref);
// }

LocalVarList FunctionInformation::getLocalVar() const { return localVariables; }

bool FunctionInformation::localVarExists(Type *T) {
  if (find_if(localVariables.begin(), localVariables.end(),
              [T](FreedStruct *fs) { return *fs == T; }) ==
      localVariables.end())
    return false;
  return true;
}

void FunctionInformation::setStructMemberFreed(FreedStruct *fstruct,
                                               int64_t num) {
  auto fs = find(freedStruct.begin(), freedStruct.end(), fstruct);
  if (fs != freedStruct.end()) {
    (*fs)->setFreedMember(num);
  }
}

void FunctionInformation::setStructMemberAllocated(FreedStruct *fstruct,
                                                   int64_t num) {
  auto fs = find(freedStruct.begin(), freedStruct.end(), fstruct);
  if (fs != freedStruct.end()) {
    (*fs)->setAllocatedMember(num);
  }
}

vector<bool> FunctionInformation::getStructMemberFreed(Type *T) {
  auto fs = find_if(freedStruct.begin(), freedStruct.end(),
                    [T](FreedStruct *f) { return *f == T; });
  if (fs != freedStruct.end()) return (*fs)->getFreedMember();
  return vector<bool>();
}

void FunctionInformation::copyStructMemberFreed(Type *T, vector<bool> members) {
  auto fs = find_if(freedStruct.begin(), freedStruct.end(),
                    [T](FreedStruct *f) { return *f == T; });
  if (fs != freedStruct.end()) {
    for (int ind = 0; ind != members.size(); ind++) {
      if (members[ind]) {
        (*fs)->setFreedMember(ind);
      }
    }
  }
}

void FunctionInformation::addBasicBlockLiveVariable(BasicBlock *B, Value *V) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) BInfo->addLiveVariable(V);
}
bool FunctionInformation::isFreedInBasicBlock(BasicBlock *B, Value *val,
                                              Type *ty, long mem) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  const UniqueKey *UK = this->getUniqueKeyManager()->getUniqueKey(val, ty, mem);
  if (BInfo && UK) return BInfo->FreeExists(UK);
  return false;
}

bool FunctionInformation::isFreedInBasicBlock(BasicBlock *B,
                                              const UniqueKey *UK) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) return BInfo->FreeExists(UK);
  return false;
}

bool FunctionInformation::isAllocatedInBasicBlock(BasicBlock *B, Value *val,
                                                  Type *ty, long mem) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  const UniqueKey *UK = this->getUniqueKeyManager()->getUniqueKey(val, ty, mem);
  if (BInfo && UK) return BInfo->AllocExists(UK);
  return false;
}

//     bool FunctionInformation::isAllocatedInBasicBlock(BasicBlock *B, Type*
//     ty){
//         BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
//         const UniqueKey *UK = this->getUniqueKeyManager()->getUniqueKey(*ty);
//         if(BInfo && UK)
//             return BInfo->AllocExists(UK);
//         return false;
//     }

bool FunctionInformation::isAllocatedInBasicBlock(BasicBlock *B,
                                                  const UniqueKey *UK) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) return BInfo->AllocExists(UK);
  return false;
}

bool FunctionInformation::isLiveInBasicBlock(BasicBlock *B, Value *val) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) return BInfo->LiveVariableExists(val);
}
void FunctionInformation::setCorrectlyBranched(BasicBlock *B) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) BInfo->setCorrectlyBranched();
}
bool FunctionInformation::isCorrectlyBranched(BasicBlock *B) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) return BInfo->isCorrectlyBranched();
  return false;
}

bool FunctionInformation::isPredBlockCorrectlyBranched(BasicBlock *B) {
  return BBManage.isPredBlockCorrectlyBranched(B);
}

void FunctionInformation::addCorrectlyFreedValue(BasicBlock *B,
                                                 const UniqueKey *UK) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) {
    BInfo->addCorrectlyFreedValue(UK);
  }
}

bool FunctionInformation::isCorrectlyBranchedFreeValue(BasicBlock *B, Value *V,
                                                       Type *T, long mem) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  const UniqueKey *UK = this->getUniqueKeyManager()->getUniqueKey(V, T, mem);
  if (BInfo && UK) return BInfo->CorrectlyFreedValueExists(UK);
  return false;
}

bool FunctionInformation::isCorrectlyBranchedFreeValue(BasicBlock *B,
                                                       const UniqueKey *UK) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) return BInfo->CorrectlyFreedValueExists(UK);
  return false;
}
void FunctionInformation::setLoopBlock(BasicBlock &B) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(&B);
  if (LoopI->getLoopFor(&B)) {
    if (BInfo) BInfo->setLoopBlock();
  }
}

bool FunctionInformation::isLoopBlock(BasicBlock &B) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(&B);
  if (BInfo) return BInfo->isLoopBlock();
  return false;
}

// void FunctionInformation::copyCorrectlyFreedValueInLoop(BasicBlock &B){
//     if(this->isLoopBlock(B)){
//         BBManage.copyCorrectlyFreedToPrev(&B);
//     }
// }

void FunctionInformation::updateSuccessorBlock(BasicBlock &B) {
  BBManage.updateSuccessorBlock(&B);
}

void FunctionInformation::setLoopInfo(LoopInfo *li) { LoopI = li; }

void FunctionInformation::setAlias(Value *src, Value *tgt) {
  if (!this->aliasExists(tgt)) {
    aliasMap[tgt] = src;
  }
}

bool FunctionInformation::aliasExists(Value *src) {
  if (aliasMap.find(src) != aliasMap.end()) return true;
  return false;
}

Value *FunctionInformation::getAlias(Value *src) {
  if (this->aliasExists(src)) {
    return aliasMap[src];
  }
  return NULL;
}

BasicBlockManager *FunctionInformation::getBasicBlockManager() {
  return &BBManage;
}

BasicBlockInformation *FunctionInformation::getBasicBlockInformation(
    BasicBlock *B) {
  return BBManage.get(B);
}

void FunctionInformation::addFunctionPointerInfo(Value *val, Function *func) {
  funcPtr[val].push_back(func);
}

vector<Function *> FunctionInformation::getPointedFunctions(Value *val) {
  return funcPtr[val];
}

void FunctionInformation::addAliasedType(Value *V, Type *T) {
  if (aliasedType.find(V) == aliasedType.end()) aliasedType[V] = T;
}

Type *FunctionInformation::getAliasedType(Value *V) {
  if (aliasedType.find(V) != aliasedType.end()) return aliasedType[V];
  return NULL;
}

bool FunctionInformation::aliasedTypeExists(Value *V) {
  if (aliasedType.find(V) != aliasedType.end()) return true;
  return false;
}

BasicBlockList FunctionInformation::getAllocatedInReturn() {
  for (BasicBlock *B : this->getEndPoint()) {
    return this->getAllocList(B);
  }
  return BasicBlockList();
}

BasicBlockList FunctionInformation::getAllocatedInSuccess() {
  for (BasicBlock *B : this->getEndPoint()) {
    return this->getAllocList(B);
  }
  return BasicBlockList();
}

BasicBlockList FunctionInformation::getAllocatedInError(int errcode) {
  BasicBlockList BBL = BasicBlockList();
  for (pair<int64_t, BasicBlock *> p : this->getErrorBlock()) {
    if (errcode == 0 || p.first == errcode) {
      BasicBlockList tmpBBL =
          diffList(getAllocList(p.second), getFreeList(p.second));
      for (auto endBlock : this->getEndPoint()) {
        // tmpBBL = diffList(tmpBBL,
        // getBasicBlockInformation(p.second)->getRemoveAllocs(endBlock).getList());
      }
      BBL = uniteList(BBL, tmpBBL);
    }
  }
  return BBL;
}

BasicBlockList FunctionInformation::getFreedInReturn() {
  for (BasicBlock *B : this->getEndPoint()) {
    return this->getFreeList(B);
  }
  return BasicBlockList();
}

BasicBlockList FunctionInformation::getFreedInSuccess() {
  for (BasicBlock *B : this->getEndPoint()) {
    return this->getFreeList(B);
  }
  return BasicBlockList();
}

BasicBlockList FunctionInformation::getFreedInError(int errcode) {
  BasicBlockList BBL = BasicBlockList();
  for (pair<int64_t, BasicBlock *> p : this->getErrorBlock()) {
    if (errcode == 0 || p.first == errcode) {
      // BasicBlockList tmpBBL = diffList(getFreeList(p.second),
      // getFreeList(p.second)); for (auto endBlock:this->getEndPoint()) {
      //     // tmpBBL = diffList(tmpBBL,
      //     getBasicBlockInformation(p.second)->getRemoveAllocs(endBlock).getList());
      // }
      // BBL = uniteList(BBL, tmpBBL);
    }
  }
  return BBL;
}

BasicBlockList FunctionInformation::getPendingStoreInReturn() {
  for (BasicBlock *B : this->getEndPoint()) {
    return this->getPendingStoreList(B);
  }
  return BasicBlockList();
}

BasicBlockList FunctionInformation::uniteList(BasicBlockList src,
                                              BasicBlockList tgt) {
  BasicBlockList tmp;
  llvm::sort(src.begin(), src.end());
  llvm::sort(tgt.begin(), tgt.end());

  set_union(src.begin(), src.end(), tgt.begin(), tgt.end(), back_inserter(tmp));
  return tmp;
}

BasicBlockList FunctionInformation::diffList(BasicBlockList src,
                                             BasicBlockList tgt) {
  BasicBlockList tmp;
  llvm::sort(src.begin(), src.end());
  llvm::sort(tgt.begin(), tgt.end());

  set_difference(src.begin(), src.end(), tgt.begin(), tgt.end(),
                 back_inserter(tmp));
  return tmp;
}
}  // namespace ST_free
