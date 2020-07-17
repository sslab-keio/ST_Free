#include "include/FreedStruct.hpp"

namespace ST_free {
void FreedStruct::print() {
  outs() << "\t[FreedMember]\n";
  for (int ind = 0; ind < FreedMembers.size(); ind++) {
    outs() << "\t  [" << ind << "] ";
    if (FreedMembers[ind])
      outs() << "IsFreed\n";
    else
      outs() << "NotFreed\n";
  }
  return;
}

}  // namespace ST_free
