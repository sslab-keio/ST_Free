#include "include/relationshipInformation.hpp"

using namespace {
    bool relationshipInformation::exists(Value *src){
        if(rmap.find(src) != rmap.end())
            return true;
        return false;
    }

    void relationshipInformation::add(Value *src, Value *tgt){
        if(!this->exists(src))
            rmap[src] = aliasList();
        rmap[src].push_back(tgt);
        return;
    }

    aliasList relationshipInformation::get(Value *src){
        if(this->exists(src))
            return rmap[src];
        return aliasList();
    }
}
