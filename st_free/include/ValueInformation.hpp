#pragma once
#include "ST_free.hpp"
#include "UniqueKeyManager.hpp"

namespace ST_free {
class ValueInformation {
 public:
  ValueInformation(Value *val) {
    V = val;
    refCount = 0;
    freed = false;
    arg_number = ROOT_INDEX;
  }
  ValueInformation(Value *val, Type *ty) {
    V = val;
    refCount = 0;
    freed = false;
    arg_number = ROOT_INDEX;
  }

  ValueInformation(Value *val, ParentList plist) {
    V = val;
    refCount = 0;
    freed = false;
    arg_number = ROOT_INDEX;
    for (auto parent : plist) parents.push_back(parent);
  }
  bool operator==(const Value *val) { return V == val; }
  bool operator<(const ValueInformation &val) {
    return this->V < val.getValue();
  }

  bool operator>(const ValueInformation &val) {
    return this->V > val.getValue();
  }
  Value *getValue() const;
  void incrementRefCount(Value *v) { refCount++; };
  void decrementRefCount() {
    if (refCount > 0) refCount--;
  };
  bool noRefCount() { return refCount == 0; };
  void setFreed() { freed = true; }
  bool isFreed() { return freed; }
  void addParent(Type *ty, int ind);
  ParentList getParents() { return parents; }
  Type *getTopParent();
  bool isArgValue() { return arg_number != ROOT_INDEX; }
  void setArgNumber(long arg) { arg_number = arg; }
  long getArgNumber() { return arg_number; }

 private:
  Value *V;
  ParentList parents;
  bool freed;
  bool parentFreed;
  int refCount;
  long arg_number;
};
class ValueManager {
 private:
  map<const UniqueKey *, ValueInformation *> vinfos;

 public:
  bool exists(const UniqueKey *UK);
  ValueInformation *getValueInfo(const UniqueKey *UK);
  void addValueInfo(const UniqueKey *UK, Value *val);
  void addValueInfo(const UniqueKey *UK, Value *val, ParentList plist);
  void print();
};
}  // namespace ST_free
