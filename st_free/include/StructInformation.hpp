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
  llvm::Function *F;
  FreedStruct *fst;

 public:
  CandidateValue(llvm::Function *func, FreedStruct *fs) {
    F = func;
    fst = fs;
  }
  FreedStruct *getFreedStruct() { return fst; };
  llvm::Function *getFunction() { return F; };
  unsigned getMemberSize() { return fst->memberSize(); };
  bool memberIsFreed(unsigned ind) { return fst->memberIsFreed(ind); };
  bool memberIsAllocated(unsigned ind) { return fst->memberIsAllocated(ind); };
  llvm::Instruction *getInstruction() { return fst->getInst(); }
  llvm::Type *getTopParent() { return fst->getTopParent(); }
  void print() { fst->print(); };
};
struct globalVarInfo {
  std::vector<std::string> dirs;
  llvm::GlobalVariable *GV;
  globalVarInfo(std::vector<std::string> d, llvm::GlobalVariable *G) {
    dirs = d;
    GV = G;
  }
};

/**
 * Class Function Ptr Map
 **/
class FunctionPtrMap {
 public:
  FunctionPtrMap(){};
  FunctionPtrMap(std::string path){};
  void addFunction(std::string path, llvm::Function *func);
  std::vector<llvm::Function *> getFunctionCandidates(std::string path);
  std::pair<std::string, std::string> decodeDirName(std::string path);

 private:
  // Map to next sub dirs
  std::map<std::string, FunctionPtrMap> sub_dirs;

  // Directory information
  std::vector<llvm::Function *> funcs;
  std::string path;
};

/**
 * Class [StructInformation]
 * keeps track of each structure information,
 * including referees, candidates, each memberstats,
 * and allocated times
 **/
class StructInformation {
 public:
  // enum StructMemberStats {
  //     ISRESPONSIBLE,
  //     ISNOTRESPONSIBLE,
  //     ISUNKNOWN,
  //     NOTPOINTERTY,
  //     UNALLOCATED,
  //     PRIMITIVE,
  //     SELF_DEREFERENCE
  // };
  StructInformation(){};
  StructInformation(llvm::StructType *st);
  std::vector<llvm::StructType *> getReferees() { return referees; }
  void addReferee(llvm::StructType *st);
  bool hasSingleReferee();
  void changeToNonRefered(llvm::StructType *StTy);
  void setMemberStatUnknown(int num);
  void setMemberStatResponsible(int num);
  void setMemberStatNotResponsible(int num);
  void setMemberStatNotAllocated(int num);
  void addCandidateValue(llvm::Function *F, FreedStruct *fs);
  void print();
  void PrintJson();
  void BuildCandidateCount();
  void checkCorrectness();
  std::vector<CandidateValue *> getCandidateValue() { return candidates; };
  void incrementFreedCount(int ind);
  void incrementAllocNum() { allocNum++; }
  bool hasNoAlloc() { return allocNum == 0; }
  void incrementStoreTotal(int ind);
  bool isNotStored(int ind);
  void incrementStoreGlobalVar(int ind);
  void addFunctionPtr(int ind, llvm::Function *func, std::string path = "");
  std::vector<llvm::Function *> getFunctionPtr(int ind, std::string path = "");
  void addGVInfo(int ind, std::vector<std::string> dirs,
                 llvm::GlobalVariable *gv);
  std::vector<globalVarInfo> getGVInfo(int ind);
  llvm::StructType *getStructType() { return strTy; }
  /*** Negative Count Related ***/
  void addNegativePoint() { negativeCount++; }
  int getNegativePoint() { return negativeCount; }
  void printStoreGlobalVar(int ind) {
    llvm::outs() << "\tTotal: " << stc[ind].total << "\n";
    llvm::outs() << "\tGV: " << stc[ind].globalVar << "\n";
  }
  /*** Status Related ***/
  bool isResponsible(int ind) {
    if (0 <= ind && ind < memberStats.size())
      return memberStats[ind] == ISRESPONSIBLE;
    return false;
  };
  bool isUnknown(int ind) {
    if (0 <= ind && ind < memberStats.size())
      return memberStats[ind] == ISUNKNOWN;
    return false;
  };
  bool isPrimitive(int ind) {
    if (0 <= ind && ind < memberStats.size())
      return memberStats[ind] == PRIMITIVE;
    return false;
  };
  bool isSelfDereference(int ind) {
    if (0 <= ind && ind < memberStats.size())
      return memberStats[ind] == SELF_DEREFERENCE;
    return false;
  };
  bool isNotPointerTy(int ind) {
    if (0 <= ind && ind < memberStats.size())
      return memberStats[ind] == NOTPOINTERTY;
    return false;
  };

 private:
  struct storeCount {
    int total;
    int globalVar;
    storeCount() {
      total = 0;
      globalVar = 0;
    }
  };

  llvm::StructType *strTy;
  std::vector<llvm::StructType *> referees;
  std::vector<int> memberStats;
  std::vector<int> freedCounts;
  std::vector<storeCount> stc;
  std::vector<FunctionPtrMap> function_ptr_map;
  std::vector<std::vector<llvm::Function *>> funcPtr;
  std::vector<std::vector<globalVarInfo>> gvinfo;
  int candidateNum;
  int negativeCount;
  unsigned int allocNum;
  std::vector<CandidateValue *> candidates;
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
  StructManager(std::vector<llvm::StructType *> st);
  StructInformation *get(llvm::StructType *strTy) { return StructInfo[strTy]; }
  TypeRelationManager *getTypeRelationManager() { return &tyRel; };
  void addCandidateValue(llvm::Function *F, llvm::StructType *strTy,
                         FreedStruct *fs);
  void addAlloc(llvm::StructType *strTy);
  void addStore(llvm::StructType *strTy, int ind);
  void addGlobalVarStore(llvm::StructType *strTy, int ind);
  bool exists(llvm::StructType *);
  void print();
  void PrintAsJson();
  void BuildCandidateCount();
  void checkCorrectness();
  void addGlobalVariableInitInfo(llvm::Module &M);
  bool structHoldsAuthority(llvm::StructType *StTy, long ind);
  void markNoAlloc();

 private:
  std::map<llvm::StructType *, StructInformation *> StructInfo;
  TypeRelationManager tyRel;
  void addReferee(llvm::StructType *referee, llvm::StructType *tgt);
  void createDependencies();
  void changeStats();
  void checkNonAllocs();
};
}  // namespace ST_free
