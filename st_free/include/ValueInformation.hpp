#pragma once
#include "ST_free.hpp"
#include "UniqueKeyManager.hpp"

namespace ST_free {
class ValueInformation {
 public:
  ValueInformation(llvm::Value *val) {
    V = val;
    refCount = 0;
    freed = false;
    arg_number = ROOT_INDEX;
  }
  ValueInformation(llvm::Value *val, llvm::Type *ty) {
    V = val;
    refCount = 0;
    freed = false;
    arg_number = ROOT_INDEX;
  }

  ValueInformation(llvm::Value *val, ParentList plist) {
    V = val;
    refCount = 0;
    freed = false;
    arg_number = ROOT_INDEX;
    for (auto parent : plist) parents.push_back(parent);
  }
  bool operator==(const llvm::Value *val) { return V == val; }
  bool operator<(const ValueInformation &val) {
    return this->V < val.getValue();
  }

  bool operator>(const ValueInformation &val) {
    return this->V > val.getValue();
  }
  llvm::Value *getValue() const;
  void incrementRefCount(llvm::Value *v) { refCount++; };
  void decrementRefCount() {
    if (refCount > 0) refCount--;
  };
  bool noRefCount() { return refCount == 0; };
  void setFreed() { freed = true; }
  bool isFreed() { return freed; }
  void addParent(llvm::Type *ty, int ind);
  ParentList getParents() { return parents; }
  llvm::Type *getTopParent();
  bool isArgValue() { return arg_number != ROOT_INDEX; }
  void setArgNumber(long arg) { arg_number = arg; }
  long getArgNumber() { return arg_number; }

 private:
  llvm::Value *V;
  ParentList parents;
  bool freed;
  bool parentFreed;
  int refCount;
  long arg_number;
};
class ValueManager {
 private:
  std::map<const UniqueKey *, ValueInformation *> vinfos;

 public:
  bool exists(const UniqueKey *UK);
  ValueInformation *getValueInfo(const UniqueKey *UK);
  void addValueInfo(const UniqueKey *UK, llvm::Value *val);
  void addValueInfo(const UniqueKey *UK, llvm::Value *val, ParentList plist);
  void print();
  size_t getSize() {return vinfos.size();}
};
}  // namespace ST_free
