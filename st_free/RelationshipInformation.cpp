#include "include/RelationshipInformation.hpp"

namespace ST_free {
bool RelationshipInformation::exists(llvm::Value *src) {
  if (rmap.find(src) != rmap.end()) return true;
  return false;
}

void RelationshipInformation::add(llvm::Value *src, llvm::Value *tgt) {
  if (!this->exists(src)) rmap[src] = aliasList();
  rmap[src].push_back(tgt);
  return;
}

aliasList RelationshipInformation::get(llvm::Value *src) {
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

std::vector<AliasElement> TypeRelationManager::getRelationshipList(
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

std::ostream &TypeRelationManager::operator<<(std::ostream &out) {
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
  llvm::outs() << "<RelationManager>\n";
  for (auto element : typeMap) {
    llvm::outs() << "src: " << *element.first.stTy << " [" << element.first.index
           << "]\n";
    for (auto srcs : element.second) {
      llvm::outs() << "\ttgt: " << *srcs.stTy << " [" << srcs.index << "]\n";
    }
  }
}
}  // namespace ST_free
