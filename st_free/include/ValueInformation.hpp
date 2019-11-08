#pragma once
#include "ST_free.hpp"
#include "UniqueKeyManager.hpp"

namespace ST_free {
    class ValueInformation {
        public:
            ValueInformation(Value * val){
                V = val;
                memberNum = ROOT_INDEX;
                memberType = val->getType();
                structType = NULL;
                refCount = 0;
                freed = false;
            }
            ValueInformation(Value * val, Type * ty){
                V = val;
                memberNum = ROOT_INDEX;
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
            ValueInformation(Value * val, Type * memType, Type * parType, long num, ParentList plist){
                V = val;
                memberNum = num;
                memberType = memType;
                structType = parType;
                refCount = 0;
                freed = false;
                for (auto parent: plist)
                    parents.push_back(parent);
                // parents = ParentList(plist);
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
            void addParent(Type* ty, int ind);
            ParentList getParents() {return parents;}
            Type* getTopParent();
        private:
            Value * V;
            long memberNum;
            ParentList parents;
            Type * memberType;
            Type * structType;
            bool freed;
            bool parentFreed;
            int refCount;
    };
    class ValueManager {
        private: 
            map<const UniqueKey *, ValueInformation *> vinfos;
        public:
            bool exists(const UniqueKey *UK);
            ValueInformation * getValueInfo(const UniqueKey *UK);
            void addValueInfo(const UniqueKey *UK, Value * val);
            void addValueInfo(const UniqueKey *UK, Value * val, Type * memType, Type * parType, long num, ParentList plist);
            void print();
    };
}
