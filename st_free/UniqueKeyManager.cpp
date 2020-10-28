#include "include/UniqueKeyManager.hpp"

namespace ST_free {
void UniqueKey::print() const {
  llvm::outs() << "== Unique Key Info ==\n";
  llvm::outs() << "[Value]: " << this->getValue() << "\n";
  llvm::outs() << "[Type]: " << *this->getType() << "\n";
  llvm::outs() << "[memberNum]: " << this->getNum() << "\n";
  llvm::outs() << "=====================\n";
}

const UniqueKey *UniqueKeyManager::addUniqueKey(llvm::Value *val, llvm::Type *ty,
                                                long mem) {
  auto uk = ukmanage.insert(UniqueKey(val, ty, mem));
  return &(*(uk.first));
}

const UniqueKey *UniqueKeyManager::getUniqueKey(llvm::Value *val, llvm::Type *ty,
                                                long mem) {
  auto found = ukmanage.find(UniqueKey(val, ty, mem));
  if (found == ukmanage.end()) return NULL;
  return &(*found);
}

// const UniqueKey* UniqueKeyManager::getUniqueKeyFromField(Type *ty, long mem){
//     auto found = ukmanage.find(pair<Type *, long>(ty, mem));
//     if(found == ukmanage.end())
//         return NULL;
//     return &(*found);
// }

void UniqueKeyManager::print() {
  for (auto uk : ukmanage) {
    uk.print();
  }
}
}  // namespace ST_free
