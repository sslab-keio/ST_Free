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

    bool ValueManager::exists(Value * val, Type * ty){
        if(vinfos.find(hashKeys(val, ty)) != vinfos.end())
            return true;
        return false;
    }

    bool ValueManager::exists(Value * val){
        if(vinfos.find(hashKeys(val, val->getType())) != vinfos.end())
            return true;
        return false;
    }

    ValueInformation * ValueManager::getValueInfo(Value *val, Type * ty){
        if(this->exists(val,ty))
            return vinfos[hashKeys(val,ty)];
        return NULL;
    }

    ValueInformation * ValueManager::getValueInfo(Value *val){
        if(this->exists(val,val->getType()))
            return vinfos[hashKeys(val,val->getType())];
        return NULL;
    }

    void ValueManager::addValueInfo(Value *val, Type * ty){
        if(!this->exists(val, ty))
            vinfos[hashKeys(val, ty)] = new ValueInformation(val);
    }

    void ValueManager::addValueInfo(Value * val){
        if(!this->exists(val, val->getType()))
            vinfos[hashKeys(val, val->getType())] = new ValueInformation(val);
        return;
    }
    void ValueManager::addValueInfo(Value * val, Type * memType, Type * parType, long num){
        if(!this->exists(val, memType))
            vinfos[hashKeys(val, memType)] = new ValueInformation(val, memType, parType, num);
        return;
    }
}
