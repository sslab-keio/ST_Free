#include "include/StructInformation.hpp"

namespace ST_free {
StructInformation::StructInformation(StructType *st) {
  int ind = 0;
  strTy = st;

  memberStats = vector<int>(st->getNumElements(), ISUNKNOWN);
  freedCounts = vector<int>(st->getNumElements(), 0);

  stc = vector<storeCount>(st->getNumElements());
  funcPtr = vector<vector<Function *>>(st->getNumElements());
  gvinfo = vector<vector<globalVarInfo>>(st->getNumElements());

  candidateNum = 0;
  allocNum = 0;
  negativeCount = 0;

  for (Type *ty : st->elements()) {
    if (!ty->isPointerTy() &&
        (!ty->isArrayTy() || !ty->getArrayElementType()->isPointerTy()))
      memberStats[ind] = NOTPOINTERTY;
    else if (get_type(ty)->isFunctionTy())
      memberStats[ind] = ISNOTRESPONSIBLE;
    else if (get_type(ty)->isIntegerTy() || get_type(ty)->isFloatTy() ||
             get_type(ty)->isDoubleTy() || get_type(ty)->isHalfTy())
      memberStats[ind] = PRIMITIVE;
    else if (st == get_type(ty))
      memberStats[ind] = SELF_DEREFERENCE;
    ind++;
  }
}

void StructInformation::BuildCandidateCount() {
  for (CandidateValue *cand : candidates) {
    for (unsigned ind = 0; ind < cand->getMemberSize(); ind++) {
      if (cand->memberIsFreed(ind)) this->incrementFreedCount(ind);
    }
  }
  return;
}

void StructInformation::checkCorrectness() {
  for (CandidateValue *cand : candidates) {
    for (unsigned ind = 0; ind < cand->getMemberSize(); ind++) {
      if (!cand->memberIsFreed(ind)) {
#ifdef STAGE_ONE
        this->checkStageOne(cand, ind);
#endif
#ifdef STAGE_TWO
        this->checkStageTwo(cand, ind);
#endif
#ifdef STAGE_PRIMITIVE
        this->checkStagePrimitive(cand, ind);
#endif
#ifdef STAGE_BIDIRECTIONAL
        this->checkStageBidirectional(cand, ind);
#endif
      }
    }
  }
  return;
}

void StructInformation::checkStageOne(CandidateValue *cand, long ind) {
  string warningStr("MEMBER NOT FREED(");
  if (this->isResponsible(ind)) {
    if (this->judgeResponsibility(ind) && !this->isAllStoreGlobalVar(ind) &&
        !this->isBidirectionalReferencing(cand, ind) &&
        cand->memberIsAllocated(ind)) {
      string message = warningStr;
      message += parseErrorMessage(this->getStructType(), ind);
      message += ")";
      generateError(cand->getInstruction(), message);
    }
  }
  return;
}

void StructInformation::checkStageTwo(CandidateValue *cand, long ind) {
  string warningStr("MEMBER NOT FREED(");
  if (this->isUnknown(ind)) {
    if (this->judgeResponsibility(ind)) {
      string message = warningStr;
      message += parseErrorMessage(this->getStructType(), ind);
      message += ")";
      generateError(cand->getInstruction(), message);
    }
  }
  return;
}

void StructInformation::checkStagePrimitive(CandidateValue *cand, long ind) {
  string warningStr("MEMBER NOT FREED(");
  if (this->isPrimitive(ind)) {
    string message = warningStr;
    message += parseErrorMessage(this->getStructType(), ind);
    message += ")";
    generateError(cand->getInstruction(), message);
  }
}

void StructInformation::checkStageBidirectional(CandidateValue *cand,
                                                long ind) {
  if (this->isBidirectionalReferencing(cand, ind)) {
    // TODO: Check authority of bidirectional referenced values
  }
}

bool StructInformation::isBidirectionalReferencing(CandidateValue *cand,
                                                   int ind) {
  ParentList parents = cand->getFreedStruct()->getParentTypes();
  Type *member = strTy->getElementType(ind);
  for (pair<Type *, int> parent : parents) {
    if (get_type(parent.first) == get_type(member)) return true;
  }
  return false;
}

bool StructInformation::judgeResponsibility(int ind) {
  int threashold = candidateNum * THREASHOLD;
  // outs() << ind << " " << threashold << " " << freedCounts[ind] << "\n";
  if (freedCounts[ind] >= threashold) return true;
  return false;
}

void StructInformation::incrementFreedCount(int ind) { freedCounts[ind]++; }

void StructInformation::addReferee(StructType *st) { referees.push_back(st); }

bool StructInformation::hasSingleReferee() { return referees.size() == 1; }

void StructInformation::setMemberStatResponsible(int num) {
  if (num < memberStats.size()) memberStats[num] = ISRESPONSIBLE;
}

void StructInformation::setMemberStatNotResponsible(int num) {
  if (num < memberStats.size()) memberStats[num] = ISNOTRESPONSIBLE;
}

void StructInformation::setMemberStatNotAllocated(int num) {
  if (num < memberStats.size()) memberStats[num] = UNALLOCATED;
}

void StructInformation::setMemberStatUnknown(int num) {
  if (num < memberStats.size()) memberStats[num] = ISUNKNOWN;
}

void StructInformation::addCandidateValue(Function *F, FreedStruct *fs) {
  candidates.push_back(new CandidateValue(F, fs));
  candidateNum++;
}

void StructInformation::incrementStoreTotal(int ind) {
  if (ind < stc.size()) stc[ind].total++;
}

bool StructInformation::isNotStored(int ind) {
  if (ind < stc.size()) return stc[ind].total != 0;
  return false;
}

void StructInformation::incrementStoreGlobalVar(int ind) {
  if (ind < stc.size()) stc[ind].globalVar++;
}

void StructInformation::print() {
  outs() << "=== StructInfo: Debug Info===\n";
  if (strTy->hasName()) outs() << "[Struct]: " << strTy->getName() << "\n";
  outs() << "[AllocNum]: " << allocNum << "\n";
  outs() << "[Referees] (TTL: " << referees.size() << ") \n";
  // for (StructType* ty : referees){
  //     outs() << "\t" << *ty << "\n";
  // }
  outs() << "[Elements]\n";
  for (int ind = 0; ind < strTy->getNumElements(); ind++) {
    outs() << "\t[" << ind << "]: " << *strTy->getElementType(ind) << " ";
    switch (memberStats[ind]) {
      case ISRESPONSIBLE:
        outs() << "ISRESPONSIBLE";
        break;
      case ISNOTRESPONSIBLE:
        outs() << "ISNOTRESPONSIBLE";
        break;
      case ISUNKNOWN:
        outs() << "ISUNKNOWN ";
        if (this->judgeResponsibility(ind))
          outs() << "(Judged Responsible)";
        else
          outs() << "(Judged Not Responsible)";
        break;
      case NOTPOINTERTY:
        outs() << "NOTPOINTERTY";
        break;
      case UNALLOCATED:
        outs() << "UNALLOCATED";
        break;
      case PRIMITIVE:
        outs() << "PRIMITIVE";
        break;
      case SELF_DEREFERENCE:
        outs() << "SELF_DEREFERENCE";
        break;
      default:
        outs() << "DEFAULT";
    }
    outs() << " [" << stc[ind].globalVar << "/" << stc[ind].total << "], "
           << freedCounts[ind] << "\n";
  }
  outs() << "[CandidateNum]: " << candidateNum << "\n";
  // outs() << "[BugCandidates]\n";
  // if(candidates.size() == 0)
  //     outs() << "Empty\n";
  // for(CandidateValue* cands: candidates){
  //     outs() << "\t[Function] " << cands->getFunction()->getName() << "\n";
  //     outs() << "\t[FreedStruct] " <<
  //     cands->getFreedStruct()->getValue()->getName() << "\n";
  //     if(cands->getFreedStruct()->getTopParent())
  //         outs() << "\t[TopParent]" <<
  //         *cands->getFreedStruct()->getTopParent() << "\n";
  //     cands->print();
  //     outs() << "******\n";
  // }
  outs() << "=============================\n";
}

// void StructInformation::printjson(){
//     if (strTy->hasName())
//         outs() << "\"Struct\": \"" << strTy->getName() << "\"\n";
//     outs() << "[Elements]\n";
//     for(int ind = 0; ind < strTy->getNumElements(); ind++){
//         outs() << "\t[" << ind << "]: " << *strTy->getElementType(ind) << "
//         "; switch(memberStats[ind]){
//             case ISRESPONSIBLE:
//                 outs() << "ISRESPONSIBLE";
//                 break;
//             case ISNOTRESPONSIBLE:
//                 outs() << "ISNOTRESPONSIBLE";
//                 break;
//             case ISUNKNOWN:
//                 outs() << "ISUNKNOWN ";
//                 if(this->judgeResponsibility(ind))
//                     outs() << "(Judged Responsible)";
//                 else
//                     outs() << "(Judged Not Responsible)";
//                 break;
//             case NOTPOINTERTY:
//                 outs() << "NOTPOINTERTY";
//                 break;
//             case UNALLOCATED:
//                 outs() << "UNALLOCATED";
//                 break;
//             case PRIMITIVE:
//                 outs() << "PRIMITIVE";
//                 break;
//             case SELF_DEREFERENCE:
//                 outs() << "SELF_DEREFERENCE";
//                 break;
//             default:
//                 outs() << "DEFAULT";
//         }
//         outs() << " [" << stc[ind].globalVar << "/" << stc[ind].total << "],
//         " << freedCounts[ind] << "\n";
//     }
// }

void StructInformation::addFunctionPtr(int ind, Function *func) {
  funcPtr[ind].push_back(func);
}

vector<Function *> StructInformation::getFunctionPtr(int ind) {
  return funcPtr[ind];
}

void StructInformation::addGVInfo(int ind, vector<string> dirs,
                                  GlobalVariable *gv) {
  gvinfo[ind].push_back(globalVarInfo(dirs, gv));
  return;
}

vector<globalVarInfo> StructInformation::getGVInfo(int ind) {
  return gvinfo[ind];
}

void StructInformation::changeToNonRefered(StructType *StTy) {
  for (int i = 0; i < this->getStructType()->getNumElements(); i++) {
    if (StTy == get_type(this->getStructType()->getElementType(i))) {
      this->setMemberStatNotAllocated(i);
    }
  }
}

/*** [Struct Manager] ***/
StructManager::StructManager(vector<StructType *> st) {
  for (StructType *StrTy : st) StructInfo[StrTy] = new StructInformation(StrTy);
  this->createDependencies();
  this->changeStats();
}

void StructManager::addReferee(StructType *tgt, StructType *referee) {
  // if(!this->exists(tgt))
  //     StructInfo[tgt] = new StructInformation(tgt);
  StructInfo[tgt]->addReferee(referee);
}

bool StructManager::exists(StructType *st) {
  if (StructInfo.find(st) != StructInfo.end()) return true;
  return false;
}

void StructManager::createDependencies() {
  queue<StructType *> struct_queue;
  for (auto Stmap : StructInfo) struct_queue.push(Stmap.first);

  while (!struct_queue.empty()) {
    for (unsigned i = 0; i < struct_queue.front()->getNumElements(); i++) {
      Type *member = struct_queue.front()->getElementType(i);
      if (auto stTy = dyn_cast<StructType>(get_type(member))) {
        if (!this->exists(stTy)) {
          StructInfo[stTy] = new StructInformation(stTy);
          struct_queue.push(stTy);
        }
        this->addReferee(stTy, struct_queue.front());
      }
    }
    struct_queue.pop();
  }
}

void StructManager::changeStats() {
  for (auto Stmap : StructInfo) {
    for (unsigned ind = 0; ind < Stmap.first->getNumElements(); ind++) {
      Type *member = Stmap.first->getElementType(ind);
      if (this->get(Stmap.first)->isUnknown(ind))
        if (auto stTy = dyn_cast<StructType>(get_type(member)))
          if (StructInfo[stTy]->hasSingleReferee())
            StructInfo[Stmap.first]->setMemberStatResponsible(ind);
    }
  }
}

void StructManager::addCandidateValue(Function *F, StructType *strTy,
                                      FreedStruct *fs) {
  if (!this->exists(strTy)) StructInfo[strTy] = new StructInformation(strTy);

  StructInfo[strTy]->addCandidateValue(F, fs);
}

void StructManager::addAlloc(StructType *strTy) {
  if (!this->exists(strTy)) StructInfo[strTy] = new StructInformation(strTy);

  StructInfo[strTy]->incrementAllocNum();
}

void StructManager::addStore(StructType *strTy, int ind) {
  if (!this->exists(strTy)) StructInfo[strTy] = new StructInformation(strTy);

  StructInfo[strTy]->incrementStoreTotal(ind);
}

void StructManager::addGlobalVarStore(StructType *strTy, int ind) {
  if (!this->exists(strTy)) StructInfo[strTy] = new StructInformation(strTy);

  StructInfo[strTy]->incrementStoreGlobalVar(ind);
}

void StructManager::print() {
  for (auto Stmap : StructInfo) {
    Stmap.second->print();
  }
}

void StructManager::BuildCandidateCount() {
  this->markNoAlloc();
  for (auto Stmap : StructInfo) {
    Stmap.second->BuildCandidateCount();
  }
}

void StructManager::checkCorrectness() {
  for (auto Stmap : StructInfo) {
    Stmap.second->checkCorrectness();
  }
}

void StructManager::checkNonAllocs() {
  for (auto Stmap : StructInfo) {
    int ind = 0;
    for (Type *member : Stmap.first->elements()) {
      if (member->isPointerTy())
        if (auto stTy = dyn_cast<StructType>(get_type(member))) {
          if (StructInfo[stTy]->hasNoAlloc()) {
            Stmap.second->setMemberStatNotAllocated(ind);
          }
        }
      ind++;
    }
  }
}

void StructManager::addGlobalVariableInitInfo(Module &M) {
  for (GlobalVariable &GV : M.globals()) {
    if (GV.getValueType()->isStructTy() && GV.hasInitializer()) {
      Constant *cnst = GV.getInitializer();
      for (int i = 0; i < GV.getValueType()->getStructNumElements(); i++) {
        if (get_type(GV.getValueType()->getStructElementType(i))
                ->isFunctionTy() &&
            cnst->getAggregateElement(i)) {
          this->get(cast<StructType>(GV.getValueType()))
              ->addFunctionPtr(i, cast<Function>(cnst->getAggregateElement(i)));
        }
      }
    }
  }
}

bool StructManager::structHoldsAuthority(StructType *StTy, long ind) {
  if (this->exists(StTy) && this->get(StTy)->isResponsible(ind)) return true;
  return false;
}

void StructManager::markNoAlloc() {
  for (auto Stmap : StructInfo) {
    if (Stmap.second->hasNoAlloc()) {
      for (StructType *StTy : Stmap.second->getReferees()) {
        if (this->exists(StTy))
          this->get(StTy)->changeToNonRefered(Stmap.first);
      }
    }
  }
}
}  // namespace ST_free
