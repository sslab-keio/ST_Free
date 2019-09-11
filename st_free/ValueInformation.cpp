#include "include/ValueInformation.hpp"

namespace ST_free{
    Value * ValueInformation::getValue() const{
        return V;
    }
    Type * ValueInformation::getStructType() const{
        return structType;
    }

    Type * ValueInformation::getMemberType() const{
        return memberType;
    }

    long ValueInformation::getMemberNum() const{
        return memberNum;
    }

    bool ValueInformation::isStructMember(){
        if (memberType == NULL && structType == NULL)
            return false;
        return true;
    }

    Type* ValueInformation::getTopParent() {
        if (!parents.empty())
            return parents[0].first;
        return NULL;
    }

    // bool ValueManager::exists(Value * val, Type * ty, long num){
    //     if(vinfos.find(UniqueKey(val, ty, num)) != vinfos.end()){
    //         return true;
    //     }
    //     return false;
    // }

    // bool ValueManager::exists(Value * val){
    //     if(vinfos.find(UniqueKey(val, val->getType(), -1)) != vinfos.end())
    //         return true;
    //     return false;
    // }

    bool ValueManager::exists(const UniqueKey *UK){
        if(vinfos.find(UK) != vinfos.end())
            return true;
        return false;
    }

    // ValueInformation * ValueManager::getValueInfo(Value *val, Type * ty, long num){
    //     if(this->exists(val,ty, num))
    //         return vinfos[UniqueKey(val, ty, num)];
    //     return NULL;
    // }

    // ValueInformation * ValueManager::getValueInfo(Value *val){
    //     if(this->exists(val,val->getType(), -1))
    //         return vinfos[UniqueKey(val,val->getType(), -1)];
    //     return NULL;
    // }

    ValueInformation * ValueManager::getValueInfo(const UniqueKey *UK){
        if(this->exists(UK))
            return vinfos[UK];
        return NULL;
    }

    // void ValueManager::addValueInfo(Value *val, Type * ty, long num){
    //     if(!this->exists(val, ty, num))
    //         vinfos[UniqueKey(val, ty, num)] = new ValueInformation(val);
    // }

    void ValueManager::addValueInfo(const UniqueKey *UK, Value * val){
        if(!this->exists(UK))
            vinfos[UK] = new ValueInformation(val);
        return;
    }
    // void ValueManager::addValueInfo(Value * val, Type * memType, Type * parType, long num){
    //     if(!this->exists(val, memType, num))
    //         vinfos[UniqueKey(val, memType, num)] = new ValueInformation(val, memType, parType, num);
    //     return;
    // }
    void ValueManager::addValueInfo(const UniqueKey *UK, Value * val, Type * memType, Type * parType, long num, ParentList plist){
        if(!this->exists(UK))
            vinfos[UK] = new ValueInformation(val, memType, parType, num, plist);
        return;
    }

    void ValueManager::print(){
        for(auto vmap: vinfos){
            (vmap.first)->print();
        }
    }
}
