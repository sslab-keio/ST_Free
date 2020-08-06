#pragma once
#include "ST_free.hpp"

namespace ST_free {
using aliasList = vector<llvm::Value *>;
struct AliasElement {
  llvm::StructType *stTy;
  int index;
  AliasElement() {
    stTy = NULL;
    index = ROOT_INDEX;
  }
  AliasElement(llvm::StructType *st, int ind) {
    stTy = st;
    index = ind;
  }
  bool operator<(const struct AliasElement &ae) const {
    if (this->stTy == ae.stTy) return this->index < ae.index;
    return this->stTy < ae.stTy;
  }
  bool operator==(const struct AliasElement &ae) const {
    return this->stTy == ae.stTy && this->index == ae.index;
  }
  void set(llvm::StructType *st, int ind) {
    stTy = st;
    index = ind;
  }
};
class TypeRelationManager {
 public:
  bool exists(AliasElement src);
  void add(AliasElement src, AliasElement tgt);
  vector<AliasElement> getRelationshipList(AliasElement src);
  bool hadRelationship(AliasElement src, AliasElement tgt);
  ostream &operator<<(ostream &out);
  void print();

 private:
  map<AliasElement, vector<AliasElement>> typeMap;
};
class RelationshipInformation {
 public:
  bool exists(llvm::Value *src);
  aliasList get(llvm::Value *src);
  void add(llvm::Value *src, llvm::Value *tgt);
  bool hasRelationship(llvm::Value *src, llvm::Value *tgt);

 private:
  map<llvm::Value *, aliasList> rmap;
};
}  // namespace ST_free
