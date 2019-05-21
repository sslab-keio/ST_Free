#include "ST_free.hpp"
#pragma once

namespace ST_free {
    class ValueInformation {
        private:
            Value * V;
            long memberNum;
            Type * memberType;
            Type * structType;
        public:
            ValueInformation(Value * val){
                V = val;
                memberNum = -1;
                memberType = NULL;
                structType = NULL;
            }
            ValueInformation(Value * val, Type * memType, Type * parType, long num){
                V = val;
                memberNum = num;
                memberType = memType;
                structType = parType;
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
    };
    using BasicBlockList = vector<ValueInformation>;

    class BasicBlockWorkList {
        private:
            BasicBlockList MarkedValues;
        public:
            BasicBlockWorkList();
            BasicBlockWorkList(const BasicBlockList);
            void add(Value * v);
            void add(Value * v, Type * memType, Type * structType, long memberNum);
            bool exists(Value * v);
            BasicBlockList getList() const;
            void setList(BasicBlockList);
            // void intersect(vector<Value *>);
    };

    class BasicBlockStat {
        private:
            BasicBlockWorkList freeList;
            BasicBlockWorkList allocList;
        public:
            BasicBlockStat();
            BasicBlockStat(const BasicBlockStat &);
            void addFree(Value *v);
            void addFree(Value * v, Type * memType, Type * structType, long memberNum);
            bool FreeExists(Value *v);
            void setFreeList(BasicBlockList);
            void addAlloc(Value *v);
            void addAlloc(Value * v, Type * memType, Type * structType, long memberNum);
            bool AllocExists(Value *v);
            void setAllocList(BasicBlockList);
            BasicBlockWorkList getWorkList(int mode) const;
    };

    class BasicBlockManager {
        private:
            map<BasicBlock *,BasicBlockStat> BBMap;
            bool exists(BasicBlock *B);
            BasicBlockStat * get(BasicBlock *B);
            BasicBlockList intersectList(BasicBlockList src, BasicBlockList tgt);
        public:
            void CollectInInfo(BasicBlock *B, bool isEntryPoint);
            void add(BasicBlock * B, Value *v, int mode);
            void add(BasicBlock * B, Value *v, Type * memTy, Type * stTy, long num, int mode);
            void copy(BasicBlock *src, BasicBlock *tgt);
            void intersect(BasicBlock *src, BasicBlock *tgt);
            BasicBlockList getBasicBlockFreeList(BasicBlock *src);
            BasicBlockList getBasicBlockAllocList(BasicBlock *src);
    };
}
