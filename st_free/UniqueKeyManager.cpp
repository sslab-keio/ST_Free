#include "include/UniqueKeyManager.hpp"

namespace ST_free {
    void UniqueKey::print() const{
        outs() << "== Unique Key Info ==\n";
        outs() << "[Value]: " << *this->getValue() << "\n";
        outs() << "[Type]: " << *this->getType() << "\n";
        outs() << "[memberNum]: " << this->getNum() << "\n";
        outs() << "=====================\n";
    }

    const UniqueKey* UniqueKeyManager::addUniqueKey(Value *val, Type *ty, long mem) {
        auto uk = ukmanage.insert(UniqueKey(val, ty, mem));
        return &(*(uk.first));
    }

    const UniqueKey* UniqueKeyManager::getUniqueKey(Value *val, Type *ty, long mem){
        auto found = ukmanage.find(UniqueKey(val, ty, mem));
        if(found == ukmanage.end())
            return NULL;
        return &(*found);
    }

    // const UniqueKey* UniqueKeyManager::getUniqueKeyFromField(Type *ty, long mem){
    //     auto found = ukmanage.find(pair<Type *, long>(ty, mem));
    //     if(found == ukmanage.end())
    //         return NULL;
    //     return &(*found);
    // }

    void UniqueKeyManager::print(){
        for (auto uk: ukmanage) {
            uk.print();
        }
    }
}
