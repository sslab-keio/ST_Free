#include "include/ValueInformation.hpp"
namespace ST_free{
    void uniqueKey::print() const{
        outs() << "== Unique Key Info ==\n";
        outs() << "[Value]: " << *this->getValue() << "\n";
        outs() << "[Type]: " << *this->getType() << "\n";
        outs() << "[memberNum]: " << this->getNum() << "\n";
        outs() << "=====================\n";
    }
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

    bool ValueManager::exists(Value * val, Type * ty, long num){
        if(vinfos.find(uniqueKey(val, ty, num)) != vinfos.end()){

            return true;
        }
        return false;
    }

    bool ValueManager::exists(Value * val){
        if(vinfos.find(uniqueKey(val, val->getType(), -1)) != vinfos.end())
            return true;
        return false;
    }

    ValueInformation * ValueManager::getValueInfo(Value *val, Type * ty, long num){
        if(this->exists(val,ty, num))
            return vinfos[uniqueKey(val, ty, num)];
        return NULL;
    }

    ValueInformation * ValueManager::getValueInfo(Value *val){
        if(this->exists(val,val->getType(), -1))
            return vinfos[uniqueKey(val,val->getType(), -1)];
        return NULL;
    }

    void ValueManager::addValueInfo(Value *val, Type * ty, long num){
        if(!this->exists(val, ty, num))
            vinfos[uniqueKey(val, ty, num)] = new ValueInformation(val);
    }

    void ValueManager::addValueInfo(Value * val){
        if(!this->exists(val, val->getType(), -1))
            vinfos[uniqueKey(val, val->getType(), -1)] = new ValueInformation(val);
        return;
    }
    void ValueManager::addValueInfo(Value * val, Type * memType, Type * parType, long num){
        if(!this->exists(val, memType, num))
            vinfos[uniqueKey(val, memType, num)] = new ValueInformation(val, memType, parType, num);
        return;
    }
    void ValueManager::print(){
        for(auto vmap: vinfos){
            vmap.first.print();
        }
    }
    void ValueManager::addAlias(Value *srcVal, Value * tgtVal){
        return;
    }
}
