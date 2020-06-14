#pragma once
#include "FunctionManager.hpp"
#include "RelationshipInformation.hpp"
#include "ST_free.hpp"
#include "determinator.hpp"
#include "support_funcs.hpp"

#define ISRESPONSIBLE 1
#define ISNOTRESPONSIBLE 2
#define ISUNKNOWN 3
#define NOTPOINTERTY 4
#define UNALLOCATED 5
#define PRIMITIVE 6
#define SELF_DEREFERENCE 7

#define THREASHOLD 0.0

namespace ST_free {
/** Class [CandidateValue]
 *
 * */
class CandidateValue {
 private:
  Function *F;
  FreedStruct *fst;

 public:
  CandidateValue(Function *func, FreedStruct *fs) {
    F = func;
    fst = fs;
  }
  FreedStruct *getFreedStruct() { return fst; };
  Function *getFunction() { return F; };
  unsigned getMemberSize() { return fst->memberSize(); };
  bool memberIsFreed(unsigned ind) { return fst->memberIsFreed(ind); };
  bool memberIsAllocated(unsigned ind) { return fst->memberIsAllocated(ind); };
  bool memberIsStoredInLoop(unsigned ind) { return fst->isStoredInLoop(ind); };
  Instruction *getInstruction() { return fst->getInst(); }
  Type *getTopParent() { return fst->getTopParent(); }
  void print() { fst->print(); };
};
struct globalVarInfo {
  vector<string> dirs;
  GlobalVariable *GV;
  globalVarInfo(vector<string> d, GlobalVariable *G) {
    dirs = d;
    GV = G;
  }
};
/**
 * Class [StructInformation]
 * keeps track of each structure information,
 * including referees, candidates, each memberstats,
 * and allocated times
 **/
class StructInformation {
 private:
  struct storeCount {
    int total;
    int globalVar;
    storeCount() {
      total = 0;
      globalVar = 0;
    }
  };
  StructType *strTy;
  vector<StructType *> referees;
  vector<int> memberStats;
  vector<int> freedCounts;
  vector<storeCount> stc;
  vector<vector<Function *>> funcPtr;
  vector<vector<globalVarInfo>> gvinfo;
  int candidateNum;
  int negativeCount;
  unsigned int allocNum;
  vector<CandidateValue *> candidates;
  bool judgeResponsibility(int ind);
  bool isBidirectionalReferencing(CandidateValue *cand, int ind);
  unsigned int getAllocNum() { return allocNum; };
  bool isAllStoreGlobalVar(int ind) {
    if (stc[ind].total > 0)
      if ((stc[ind].total - stc[ind].globalVar) == 0) return true;
    return false;
  }
  bool memberTypeMatchesStructType(int ind) {
    if (strTy == strTy->getStructElementType(ind)) return true;
    return false;
  }
  void checkStageOne(CandidateValue *cand, long ind);
  void checkStageTwo(CandidateValue *cand, long ind);
  void checkStagePrimitive(CandidateValue *cand, long ind);
  void checkStageBidirectional(CandidateValue *cand, long ind);

 public:
  StructInformation(){};
  StructInformation(StructType *st);
  vector<StructType *> getReferees() { return referees; }
  void addReferee(StructType *st);
  bool hasSingleReferee();
  void changeToNonRefered(StructType *StTy);
  void setMemberStatUnknown(int num);
  void setMemberStatResponsible(int num);
  void setMemberStatNotResponsible(int num);
  void setMemberStatNotAllocated(int num);
  void addCandidateValue(Function *F, FreedStruct *fs);
  void print();
  void PrintJson();
  void BuildCandidateCount();
  void checkCorrectness();
  vector<CandidateValue *> getCandidateValue() { return candidates; };
  void incrementFreedCount(int ind);
  void incrementAllocNum() { allocNum++; }
  bool hasNoAlloc() { return allocNum == 0; }
  void incrementStoreTotal(int ind);
  bool isNotStored(int ind);
  void incrementStoreGlobalVar(int ind);
  void addFunctionPtr(int ind, Function *func);
  vector<Function *> getFunctionPtr(int ind);
  void addGVInfo(int ind, vector<string> dirs, GlobalVariable *gv);
  vector<globalVarInfo> getGVInfo(int ind);
  StructType *getStructType() { return strTy; }
  /*** Negative Count Related ***/
  void addNegativePoint() { negativeCount++; }
  int getNegativePoint() { return negativeCount; }
  void printStoreGlobalVar(int ind) {
    outs() << "\tTotal: " << stc[ind].total << "\n";
    outs() << "\tGV: " << stc[ind].globalVar << "\n";
  }
  /*** Status Related ***/
  bool isResponsible(int ind) { return memberStats[ind] == ISRESPONSIBLE; };
  bool isUnknown(int ind) { return memberStats[ind] == ISUNKNOWN; };
  bool isPrimitive(int ind) { return memberStats[ind] == PRIMITIVE; };
  bool isSelfDereference(int ind) {
    return memberStats[ind] == SELF_DEREFERENCE;
  };
};

/** Class
 * [Struct Manager]
 * Manages StructInformation in a map, and controls them
 * This manager should be generated per module.
 * The constructor generates the map of the struct, and also
 * stores the referees of each StructType.
 **/
class StructManager {
 public:
  StructManager(){};
  StructManager(vector<StructType *> st);
  StructInformation *get(StructType *strTy) { return StructInfo[strTy]; }
  TypeRelationManager *getTypeRelationManager() { return &tyRel; };
  void addCandidateValue(Function *F, StructType *strTy, FreedStruct *fs);
  void addAlloc(StructType *strTy);
  void addStore(StructType *strTy, int ind);
  void addGlobalVarStore(StructType *strTy, int ind);
  void print();
  void PrintAsJson();
  void BuildCandidateCount();
  void checkCorrectness();
  void addGlobalVariableInitInfo(Module &M);
  bool structHoldsAuthority(StructType *StTy, long ind);
  void markNoAlloc();

 private:
  map<StructType *, StructInformation *> StructInfo;
  TypeRelationManager tyRel;
  void addReferee(StructType *referee, StructType *tgt);
  void createDependencies();
  void changeStats();
  bool exists(StructType *);
  void checkNonAllocs();
};
}  // namespace ST_free
