#include "ST_free.hpp"
#include "UniqueKeyManager.hpp"
#pragma once

namespace ST_free{
    class ValueInformation {
        private:
            Value * V;
            long memberNum;
            Type * memberType;
            Type * structType;
            bool freed;
            bool parentFreed;
            int refCount;
        public:
            ValueInformation(Value * val){
                V = val;
                memberNum = -1;
                memberType = val->getType();
                structType = NULL;
                refCount = 0;
                freed = false;
            }
            ValueInformation(Value * val, Type * ty){
                V = val;
                memberNum = -1;
                memberType = ty;
                structType = NULL;
                refCount = 0;
                freed = false;
            }
            ValueInformation(Value * val, Type * memType, Type * parType, long num){
                V = val;
                memberNum = num;
                memberType = memType;
                structType = parType;
                refCount = 0;
                freed = false;
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
            void incrementRefCount(Value * v){refCount++;};
            void decrementRefCount(){if(refCount > 0)refCount--;};
            bool noRefCount(){return refCount == 0;};
            void setStructType(Type *T){structType = T;};
            void setMemberNum(long num){memberNum = num;};
            void addStructParams(Type *T, long num){
                if(T) setStructType(T);
                if(num >= 0) setMemberNum(num);
            }
            void setFreed(){freed = true;}
            bool isFreed(){return freed;}
    };
    class ValueManager{
        private: 
            map<const UniqueKey *, ValueInformation *> vinfos;
            // map<UniqueKey, ValueInformation *> vinfos;
        public:
            // bool exists(Value *val, Type * ty, long num);
            // bool exists(Value *val);
            bool exists(const UniqueKey *UK);
            // ValueInformation * getValueInfo(Value *val, Type * ty, long num);
            // ValueInformation * getValueInfo(Value *val);
            ValueInformation * getValueInfo(const UniqueKey *UK);
            // void addValueInfo(Value * val, Type * ty, long num);
            void addValueInfo(const UniqueKey *UK, Value * val);
            // void addValueInfo(Value * val, Type * memType, Type * parType, long num);
            void addValueInfo(const UniqueKey *UK, Value * val, Type * memType, Type * parType, long num);
            void print();
    };
}
