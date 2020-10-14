#include "include/LoopManager.hpp"

namespace ST_free {
bool LoopManager::exists(llvm::Function *F) {
  if (loopinfo.find(F) != loopinfo.end()) return true;
  return false;
}
void LoopManager::add(llvm::Function *F, llvm::LoopInfo li) {
  loopinfo[F] = std::move(li);
  // for(llvm::LoopInfo::iterator i = li->begin(), e=li->end(); i != e; ++i){
  //   llvm::outs() << "[LOOP] iterating over loops\n";
  // }
}

llvm::LoopInfo *LoopManager::get(llvm::Function *F) {
  if (this->exists(F)) return &loopinfo[F];
  return NULL;
}

bool LoopManager::IsInLoop(llvm::Function *F, llvm::BasicBlock *B) {
  if (this->exists(F)) { if(loopinfo[F].getLoopFor(B) != NULL) { return true; }
  }
  return false;
}

bool LoopManager::IsHeaderBlock(llvm::Function *F, llvm::BasicBlock *B) {
  if (this->exists(F)) {
    return loopinfo[F].isLoopHeader(B);
  }
  return false;
}

bool LoopManager::IsPreheaderBlock(llvm::Function *F, llvm::BasicBlock *B) {
  if (this->exists(F)) {
    for (auto LI : loopinfo[F]) {
      if(LI->getLoopPreheader() == B)
        return true;
    }
  }
  return false;
}

llvm::Loop *LoopManager::getLoop(llvm::Function *F, llvm::BasicBlock *B) {
  if (this->exists(F)) {
    if(llvm::Loop* LI = loopinfo[F].getLoopFor(B))
      return LI;
  }
  return NULL;
}
}  // namespace ST_free
