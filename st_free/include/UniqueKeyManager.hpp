#include "ST_free.hpp"
#include "support_funcs.hpp"
#pragma once

using namespace std;
using namespace llvm;

namespace ST_free {
class UniqueKey {
 private:
  Value *v;
  Type *t;
  long memberNum;

 public:
  Value *getValue() const { return v; }
  Type *getType() const { return t; }
  long getNum() const { return memberNum; }
  UniqueKey(Value *val, Type *ty, long mem) {
    v = val;
    t = ty;
    memberNum = mem;
  }
  void print() const;
  bool operator==(const UniqueKey &uk) const {
    return (v == uk.getValue() && t == uk.getType() &&
            memberNum == uk.getNum());
  }
  bool operator<(const UniqueKey &uk) const {
    if (v == uk.getValue()) {
      if (t == uk.getType())
        return memberNum < uk.getNum();
      else
        return t < uk.getType();
    }
    return v < uk.getValue();
  }
  bool operator<(const Type &T) const { return t < &T; }
  bool operator<(Type *T) const { return t < T; }
  bool operator<(const pair<Type *, long> &field) {
    if (t < field.first) {
      return memberNum < field.second;
    }
    return t < field.first;
  }
  bool fieldMatch(const UniqueKey &uk) {
    if (t == uk.getType()) return memberNum < uk.getNum();
    return t < uk.getType();
  }
};
class UniqueKeyManager {
 private:
  set<UniqueKey> ukmanage;

 public:
  const UniqueKey *addUniqueKey(Value *val, Type *ty, long mem);
  const UniqueKey *getUniqueKey(Value *val, Type *ty, long mem);
  const UniqueKey *getUniqueKeyFromField(Type *ty, long mem);
  // const UniqueKey* getUniqueKey(Type *ty);
  void print();
};
}  // namespace ST_free
