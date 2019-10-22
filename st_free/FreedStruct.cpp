#include "include/FreedStruct.hpp"

namespace ST_free {
    void FreedStruct::print() {
        outs() << "\t[FreedMember]\n";
        for(int ind = 0; ind < FreedMembers.size(); ind++){
            outs() << "\t  [" << ind << "] ";
            if(FreedMembers[ind])
                outs() << "IsFreed\n";
            else
                outs() << "NotFreed\n";
        }
        return;
    }

    void FreedStruct::setStoredInLoop(int ind) {
        if(ind < storedInLoop.size())
            storedInLoop[ind] = true;
    }

    bool FreedStruct::isStoredInLoop(int ind) {
        if(ind < storedInLoop.size())
            return storedInLoop[ind];
        return false;
    }
}

