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
    stat = AnalysisStat::UNANALYZED;
  }
}

FunctionInformation::FunctionInformation() { stat = UNANALYZED; }

Function &FunctionInformation::getFunction() { return (Function &)(*this->F); }

FunctionInformation::AnalysisStat FunctionInformation::getStat() {
  return this->stat;
}

void FunctionInformation::setStat(AnalysisStat stat) { this->stat = stat; }

void FunctionInformation::addEndPoint(BasicBlock *B, ReturnInst *RI) {
  endPoint = B;
  retInst = RI;
}

void FunctionInformation::addSuccessBlockInformation(BasicBlock *B) {
  if (!errorCodeExists(0)) {
    info_per_error_code[0] =
        InformationPerErrorCode(getAllocList(B), getFreeList(B));
  } else {
    addErrorBlockAllocInformation(0, getAllocList(B));
    addErrorBlockFreeInformation(0, getFreeList(B));
  }
}

void FunctionInformation::addErrorBlockInformation(int64_t errcode,
                                                   BasicBlock *B) {
  if (!errorCodeExists(errcode)) {
    info_per_error_code[errcode] =
        InformationPerErrorCode(getAllocList(B), getFreeList(B));
  } else {
    addErrorBlockAllocInformation(errcode, getAllocList(B));
    addErrorBlockFreeInformation(errcode, getFreeList(B));
  }
}

void FunctionInformation::addErrorBlockAllocInformation(int64_t errcode,
                                                        BasicBlockList BList) {
  if (!errorCodeExists(errcode)) {
    info_per_error_code[errcode] =
        InformationPerErrorCode(BList, BasicBlockList());
  } else {
    info_per_error_code[errcode].alloc_list =
        BasicBlockListOperation::uniteList(
            info_per_error_code[errcode].alloc_list, BList);
  }
}

void FunctionInformation::addErrorBlockFreeInformation(int64_t errcode,
                                                       BasicBlockList BList) {
  if (!errorCodeExists(errcode)) {
    info_per_error_code[errcode] =
        InformationPerErrorCode(BasicBlockList(), BList);
  } else {
    info_per_error_code[errcode].free_list =
        BasicBlockListOperation::intersectList(
            info_per_error_code[errcode].free_list, BList);
  }
}

ValueInformation *FunctionInformation::addFreeValue(BasicBlock *B, Value *V,
                                                    Type *memTy, long num,
                                                    ParentList plist) {
  const UniqueKey *UK =
      this->getUniqueKeyManager()->getUniqueKey(V, memTy, num);
  if (UK == NULL) UK = this->getUniqueKeyManager()->addUniqueKey(V, memTy, num);

  ValueInformation *varinfo = this->getValueInfo(UK);
  if (varinfo == NULL) varinfo = this->addVariable(UK, V, plist);
  varinfo->setFreed();

  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) {
    BInfo->addFree(UK);
    if (BBManage.isPredBlockCorrectlyBranched(B)) {
      this->addCorrectlyFreedValue(B, UK);
    }
  }
  return varinfo;
}

ValueInformation *FunctionInformation::addFreeValueFromDifferentFunction(
    BasicBlock *B, ValueInformation *VI, UniqueKey *UK) {
  if (VI != NULL)
    return this->addFreeValue(B, VI->getValue(), UK->getType(), UK->getNum(),
                              VI->getParents());
  return NULL;
}

void FunctionInformation::addFreeValue(BasicBlock *B, UniqueKey *UK) {
  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) BInfo->addFree(UK);
}

const UniqueKey *FunctionInformation::addAllocValue(BasicBlock *B, Value *V,
                                                    Type *T, long mem) {
  const UniqueKey *UK = this->getUniqueKeyManager()->getUniqueKey(V, T, mem);
  if (UK == NULL) UK = this->getUniqueKeyManager()->addUniqueKey(V, T, mem);

  BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
  if (BInfo) BInfo->addAlloc(UK);
  return UK;
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
  return stat == AnalysisStat::UNANALYZED ? true : false;
}

bool FunctionInformation::isInProgress() {
  return (stat == AnalysisStat::IN_PROGRESS || stat == AnalysisStat::DIRTY)
             ? true
             : false;
}

bool FunctionInformation::isDirty() {
  return stat == AnalysisStat::DIRTY ? true : false;
}

bool FunctionInformation::isAnalyzed() {
  return stat == AnalysisStat::ANALYZED ? true : false;
}

void FunctionInformation::setAnalyzed() { setStat(AnalysisStat::ANALYZED); }

void FunctionInformation::setInProgress() {
  setStat(AnalysisStat::IN_PROGRESS);
}

void FunctionInformation::setDirty() { setStat(AnalysisStat::DIRTY); }

void FunctionInformation::BBCollectInfo(BasicBlock &B, bool isEntryPoint) {
  BBManage.CollectInInfo(&B, isEntryPoint, this->getUniqueKeyAliasMap());
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
  if (find_if(freedStruct.begin(), freedStruct.end(),
              [fst](const FreedStruct *v) {
                return fst->getValue() == v->getValue() &&
                       fst->getType() == v->getType() &&
                       fst->getInst() == v->getInst();
              }) != freedStruct.end())
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

BasicBlock *FunctionInformation::getEndPoint() { return endPoint; }

ReturnInst *FunctionInformation::getReturnInst() { return retInst; }

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

long FunctionInformation::getArgIndex(Value *v) {
  return args.getOperandNum(v);
}

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
                                                   Value *val,
                                                   ParentList plist) {
  if (!VManage.exists(UK)) VManage.addValueInfo(UK, val, plist);
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
  return false;
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

void FunctionInformation::updateSuccessorBlock(BasicBlock &B) {
  BBManage.updateSuccessorBlock(&B);
}

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
  return this->getAllocList(this->getEndPoint());
}

BasicBlockList FunctionInformation::getAllocatedInSuccess() {
  return info_per_error_code[0].alloc_list;
}

BasicBlockList FunctionInformation::getAllocatedInError(
    int errcode, ErrorCollectionMethod method) {
  BasicBlockList collected_info;
  for (auto code_info : info_per_error_code) {
    if ((method == EQUALS && code_info.first == errcode) ||
        (method == LESS_THAN && code_info.first < errcode)) {
      code_info.second.alloc_list = BasicBlockListOperation::uniteList(
          collected_info, code_info.second.alloc_list);
    }
  }
  return collected_info;
}

bool FunctionInformation::errorCodeExists(int errcode) {
  if (info_per_error_code.find(errcode) != info_per_error_code.end())
    return true;
  return false;
}

bool FunctionInformation::errorCodeLessThanExists(int errcode) {
  if (find_if(info_per_error_code.begin(), info_per_error_code.end(),
              [errcode](const pair<int64_t, InformationPerErrorCode> info) {
                return info.first < errcode;
              }) != info_per_error_code.end())
    return true;
  return false;
}

BasicBlockList FunctionInformation::getFreedInReturn() {
  return this->getFreeList(this->getEndPoint());
}

BasicBlockList FunctionInformation::getFreedInSuccess() {
  return info_per_error_code[0].free_list;
}

BasicBlockList FunctionInformation::getFreedInError(
    int errcode, ErrorCollectionMethod method) {
  bool first = true;
  BasicBlockList collected_info;

  for (auto code_info : info_per_error_code) {
    if ((method == EQUALS && code_info.first == errcode) ||
        (method == LESS_THAN && code_info.first < errcode)) {
      if (first) {
        collected_info = code_info.second.free_list;
        first = false;
      } else {
        code_info.second.free_list = BasicBlockListOperation::intersectList(
            collected_info, code_info.second.free_list);
      }
    }
  }
  return collected_info;
}

BasicBlockList FunctionInformation::getPendingStoreInReturn() {
  return this->getPendingStoreList(this->getEndPoint());
}

void FunctionInformation::setUniqueKeyAlias(const UniqueKey *src,
                                            const UniqueKey *dest) {
  if (allocated_alias.find(src) == allocated_alias.end()) {
    allocated_alias[src] = dest;
  } else {
    // TODO: Has more than one alias. Think about what needs to be done.
  }
}

const map<const UniqueKey *, const UniqueKey *>
    *FunctionInformation::getUniqueKeyAliasMap() {
  return &allocated_alias;
}

}  // namespace ST_free
