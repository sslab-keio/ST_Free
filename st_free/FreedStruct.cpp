#include "include/FreedStruct.hpp"

namespace ST_free {
void FreedStruct::print() {
  llvm::outs() << "\t[FreedMember]\n";
  for (int ind = 0; ind < FreedMembers.size(); ind++) {
    llvm::outs() << "\t  [" << ind << "] ";
    if (FreedMembers[ind])
      llvm::outs() << "IsFreed\n";
    else
      llvm::outs() << "NotFreed\n";
  }
  return;
}

}  // namespace ST_free
