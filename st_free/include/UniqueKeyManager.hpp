#include "ST_free.hpp"
#include "support_funcs.hpp"
#pragma once

namespace ST_free {
class UniqueKey {
 public:
  llvm::Value *getValue() const { return v; }
  llvm::Type *getType() const { return t; }
  long getNum() const { return memberNum; }
  UniqueKey(llvm::Value *val, llvm::Type *ty, long mem) {
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
  bool operator<(const llvm::Type &T) const { return t < &T; }
  bool operator<(llvm::Type *T) const { return t < T; }
  bool operator<(const std::pair<llvm::Type *, long> &field) {
    if (t < field.first) {
      return memberNum < field.second;
    }
    return t < field.first;
  }
  bool fieldMatch(const UniqueKey &uk) {
    if (t == uk.getType()) return memberNum < uk.getNum();
    return t < uk.getType();
  }

 private:
  llvm::Value *v;
  llvm::Type *t;
  long memberNum;
};

class UniqueKeyManager {
 private:
  std::set<UniqueKey> ukmanage;

 public:
  const UniqueKey *addUniqueKey(llvm::Value *val, llvm::Type *ty, long mem);
  const UniqueKey *getUniqueKey(llvm::Value *val, llvm::Type *ty, long mem);
  const UniqueKey *checkAndAddUniqueKey(llvm::Value *val, llvm::Type *ty, long mem);
  const UniqueKey *getUniqueKeyFromField(llvm::Type *ty, long mem);
  // const UniqueKey* getUniqueKey(Type *ty);
  void print();
};
}  // namespace ST_free
