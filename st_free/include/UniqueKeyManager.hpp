#include "ST_free.hpp"
#pragma once

using namespace std;
using namespace llvm;

namespace ST_free {
    class UniqueKey {
        private:
            Value * v;
            Type * t;
            long memberNum;
        public:
            Value * getValue() const {return v;}
            Type * getType() const {return t;}
            long getNum() const {return memberNum;}
            UniqueKey(Value *val, Type *ty, long mem){v = val;t = ty;memberNum = mem;}
            void print() const;
            bool operator ==(const UniqueKey& uk) const {return (v == uk.getValue() && t == uk.getType() && memberNum == uk.getNum());}
            bool operator<(const UniqueKey& uk) const {
                if(v == uk.getValue()){
                    if(t == uk.getType()) return memberNum < uk.getNum();
                    else return t < uk.getType();
                }
                return v < uk.getValue();
            }
    };
    class UniqueKeyManager {
        private:
            set<UniqueKey> ukmanage;
        public:
            const UniqueKey* addUniqueKey(Value *val, Type *ty, long mem);
            const UniqueKey* getUniqueKey(Value *val, Type *ty, long mem); 
            void print();
    };
}
