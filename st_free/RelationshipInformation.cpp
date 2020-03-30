#include "include/RelationshipInformation.hpp"

namespace ST_free {
bool RelationshipInformation::exists(Value *src) {
  if (rmap.find(src) != rmap.end()) return true;
  return false;
}

void RelationshipInformation::add(Value *src, Value *tgt) {
  if (!this->exists(src)) rmap[src] = aliasList();
  rmap[src].push_back(tgt);
  return;
}

aliasList RelationshipInformation::get(Value *src) {
  if (this->exists(src)) return rmap[src];
  return aliasList();
}

bool TypeRelationManager::exists(AliasElement src) {
  if (typeMap.find(src) != typeMap.end()) return true;
  return false;
}

void TypeRelationManager::add(AliasElement src, AliasElement tgt) {
  typeMap[src].push_back(tgt);
  typeMap[tgt].push_back(src);
}

vector<AliasElement> TypeRelationManager::getRelationshipList(
    AliasElement src) {
  return typeMap[src];
}

bool TypeRelationManager::hadRelationship(AliasElement src, AliasElement tgt) {
  if (this->exists(src))
    if (find(typeMap[src].begin(), typeMap[src].end(), tgt) !=
        typeMap[src].end())
      return true;
  return false;
}

ostream &TypeRelationManager::operator<<(ostream &out) {
  out << "<RelationManager>\n";
  for (auto element : typeMap) {
    out << "src: " << element.first.stTy << " [" << element.first.index
        << "]\n";
    for (auto srcs : element.second) {
      out << "\ttgt: " << srcs.stTy << " [" << srcs.index << "]\n";
    }
  }
}

void TypeRelationManager::print() {
  outs() << "<RelationManager>\n";
  for (auto element : typeMap) {
    outs() << "src: " << *element.first.stTy << " [" << element.first.index
           << "]\n";
    for (auto srcs : element.second) {
      outs() << "\ttgt: " << *srcs.stTy << " [" << srcs.index << "]\n";
    }
  }
}
}  // namespace ST_free
