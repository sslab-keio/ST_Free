#include "ST_free.hpp"
#pragma once

namespace ST_free{
    class uniqueKey {
        private:
            Value * v;
            Type * t;
            long memberNum;
        public:
            Value * getValue() const {return v;}
            Type * getType() const {return t;}
            long getNum() const {return memberNum;}
            uniqueKey(Value *val, Type *ty, long mem){v = val;t = ty;memberNum = mem;}
            void print() const;
            bool operator ==(const uniqueKey& uk) const {return (v == uk.getValue() && t == uk.getType() && memberNum == uk.getNum());}
            bool operator<(const uniqueKey& uk) const {
                if(v == uk.getValue()){
                    if(t == uk.getType()) return memberNum < uk.getNum();
                    else return t < uk.getType();
                }
                return v < uk.getValue();
            }
    };
    // struct keyCompare {
    //    bool operator() (const uniqueKey& lhs, const uniqueKey& rhs) const
    //    {
    //        return (lhs.v < rhs.v || lhs.t < rhs.t || lhs.memberNum < rhs.memberNum);
    //    }
    // };

    // using uniqueKey = pair<Value *, pair<Type *, long >>;
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
            ValueInformation(Value * val, Type * ty){
                V = val;
                memberNum = -1;
                memberType = ty;
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
            void setStructType(Type *T){structType = T;};
            void setMemberNum(long num){memberNum = num;};
            void addStructParams(Type *T, long num){setStructType(T);setMemberNum(num);}; 
            vector<Value *> getReferees() const {return referees;};
    };
    class ValueManager{
        private:
            map<uniqueKey, ValueInformation *> vinfos;
            map<ValueInformation *, ValueInformation *> alias;
        public:
            bool exists(Value *val, Type * ty, long num);
            bool exists(Value *val);
            ValueInformation * getValueInfo(Value *val, Type * ty, long num);
            ValueInformation * getValueInfo(Value *val);
            void addValueInfo(Value * val, Type * ty, long num);
            void addValueInfo(Value * val);
            void addValueInfo(Value * val, Type * memType, Type * parType, long num);
            void addAlias(Value * srcVal, Value * tgtVal); // tgtVal <- srcVal;
            void print();
    };
}
