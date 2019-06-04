#include "ST_free.hpp"
#pragma once

namespace ST_free{
    class ValueInformation {
        private:
            Value * V;
            long memberNum;
            Type * memberType;
            Type * structType;
            bool parentFreed;
            int refCount;
            vector<Value *> referees;
        public:
            ValueInformation(Value * val){
                V = val;
                memberNum = -1;
                memberType = val->getType();
                structType = NULL;
                refCount = 0;
            }
            ValueInformation(Value * val, Type * memType, Type * parType, long num){
                V = val;
                memberNum = num;
                memberType = memType;
                structType = parType;
                refCount = 0;
            }
            bool operator == (const Value * val){
                return V == val;
            }
            bool operator == (const Type * strType){
                return structType == strType;
            }
            bool operator < (const ValueInformation & val){
                return this->V < val.getValue();
            }

            bool operator > (const ValueInformation & val){
                return this->V > val.getValue();
            }
            Value * getValue() const;
            Type * getStructType() const;
            Type * getMemberType() const;
            long getMemberNum() const;
            bool isStructMember();
            void incrementRefCount(Value * v){refCount++; referees.push_back(v);};
            void decrementRefCount(){if(refCount > 0)refCount--;};
            bool noRefCount(){return refCount == 0;};
            void addRefereeValue(Value *);
    };
}
