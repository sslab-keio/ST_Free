#include "include/LoopManager.hpp"

namespace ST_free {
bool LoopManager::exists(Function *F) {
  if (loopinfo.find(F) != loopinfo.end()) return true;
  return false;
}
void LoopManager::add(Function *F, LoopInfo *li) { loopinfo[F] = li; }

LoopInfo *LoopManager::get(Function *F) {
  if (this->exists(F)) return loopinfo[F];
  return NULL;
}
}  // namespace ST_free
