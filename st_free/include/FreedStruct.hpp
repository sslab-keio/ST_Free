#pragma once
#include "ST_free.hpp"
#include "ValueInformation.hpp"

namespace ST_free {
struct FreedStruct {
 public:
  FreedStruct(){};
  FreedStruct(llvm::Type *Ty, llvm::Value *val, llvm::Instruction *Inst) {
    T = Ty;
    V = val;
    I = Inst;
    FreedMembers = std::vector<bool>(Ty->getStructNumElements(), false);
  };
  FreedStruct(llvm::Type *Ty, llvm::Value *val, llvm::Instruction *Inst, ParentList P) {
    T = Ty;
    V = val;
    I = Inst;
    FreedMembers = std::vector<bool>(Ty->getStructNumElements(), false);
    ParentType = ParentList(P);
    valInfo = NULL;
  };
  FreedStruct(llvm::Type *Ty, llvm::Value *val, llvm::Instruction *Inst, llvm::BasicBlock *freedB,
              ValueInformation *vinfo, bool hasParent = false) {
    T = Ty;
    V = val;
    I = Inst;
    FreedMembers = std::vector<bool>(Ty->getStructNumElements(), false);
    AllocatedMembers = std::vector<bool>(Ty->getStructNumElements(), false);
    valInfo = vinfo;
    freedBlock = freedB;
    inStruct = hasParent;
  };
  FreedStruct(llvm::Type *Ty, llvm::Value *val, llvm::Instruction *Inst, ParentList P,
              llvm::BasicBlock *freedB, ValueInformation *vinfo,
              bool hasParent = false) {
    T = Ty;
    V = val;
    I = Inst;
    FreedMembers = std::vector<bool>(Ty->getStructNumElements(), false);
    AllocatedMembers = std::vector<bool>(Ty->getStructNumElements(), false);
    valInfo = vinfo;
    freedBlock = freedB;
    inStruct = hasParent;
  };
  bool operator==(llvm::Value *v) { return V == v; }
  bool operator==(llvm::Type *t) { return this->T == t; }
  bool operator==(llvm::Type t) { return this->T == &t; }
  bool operator==(FreedStruct *v) {
    return V == v->getValue() && T == v->getType() && I == v->getInst();
  }
  bool operator!=(FreedStruct *v) {
    return V != v->getValue() && T != v->getType() && I != v->getInst();
  }
  bool operator==(FreedStruct v) {
    return V == v.getValue() && T == v.getType() && I == v.getInst();
  }
  bool operator!=(FreedStruct v) {
    return V != v.getValue() && T != v.getType() && I != v.getInst();
  }
  bool operator==(UniqueKey uk) {
    return T == uk.getType() && V == uk.getValue();
  }
  bool operator!=(UniqueKey uk) {
    return T != uk.getType() && V != uk.getValue();
  }
  bool memberIsFreed(int ind) {
    return ind < FreedMembers.size() ? FreedMembers[ind] : false;
  }
  bool memberIsAllocated(int ind) {
    return ind < AllocatedMembers.size() ? AllocatedMembers[ind] : false;
  }
  unsigned memberSize() { return FreedMembers.size(); }
  void print();
  /*** getter/setter ***/
  llvm::Type *getType() const { return T; }
  llvm::Value *getValue() const { return V; }
  llvm::Instruction *getInst() const { return I; }
  ValueInformation *getValueInformation() { return valInfo; }
  void setFreedMember(int64_t num) { FreedMembers[num] = true; };
  void setAllocatedMember(int64_t num) { AllocatedMembers[num] = true; };
  std::vector<bool> getFreedMember() const { return FreedMembers; };
  std::vector<bool> getAllocatedMember() const { return AllocatedMembers; };
  llvm::BasicBlock *getFreedBlock() const { return freedBlock; };
  void addParent(llvm::StructType *st, int ind) {
    ParentType.push_back(std::pair<llvm::Type *, int>(st, ind));
  }
  // Type *getTopParent(){return this->getValueInformation()->getTopParent();}
  llvm::Type *getTopParent() {
    return ParentType.empty() ? NULL : ParentType[0].first;
  }
  ParentList getParentTypes() {
    return this->getValueInformation()->getParents();
  }
  bool isInStruct() { return inStruct; }

 private:
  llvm::Type *T;
  ParentList ParentType;
  llvm::Value *V;
  llvm::Instruction *I;
  std::vector<bool> FreedMembers;
  std::vector<bool> AllocatedMembers;
  ValueInformation *valInfo;
  llvm::BasicBlock *freedBlock;
  bool inStruct;
};
}  // namespace ST_free
