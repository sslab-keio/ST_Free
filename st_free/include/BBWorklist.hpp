#include "ST_free.hpp"
#include "ValueInformation.hpp"
#pragma once

namespace ST_free {
    using BasicBlockList = vector<pair<Value *, Type *>>;
    // using BasicBlockList = vector<ValueInformation>;
    class BasicBlockWorkList {
        private:
            using hashKeys = pair<Value *, Type *>;
            BasicBlockList MarkedValues;
        public:
            BasicBlockWorkList();
            BasicBlockWorkList(const BasicBlockList);
            void add(Value * v, Type * t);
            // void add(Value * v, Type * memType, Type * structType, long memberNum);
            bool exists(Value * v, Type *t);
            // void incrementRefCount(Value *v, Value * refVal);
            // void decrementRefCount(Value *v, Value * refVal);
            BasicBlockList getList() const;
            void setList(BasicBlockList);
            // void intersect(vector<Value *>);
    };

    using LiveVariableList = vector<Value *>;
    class BasicBlockStat {
        private:
            BasicBlockWorkList freeList;
            BasicBlockWorkList allocList;
            LiveVariableList liveVariables;
        public:
            BasicBlockStat();
            BasicBlockStat(const BasicBlockStat &);
            /*** Free Related Methods ***/
            // void addFree(Value *v);
            void addFree(Value * v, Type * ty);
            bool FreeExists(Value *v, Type * ty);
            void setFreeList(BasicBlockList);
            // void incrementFreedRefCount(Value *v, Value * refVal);
            // void decrementFreedRefCount(Value *v, Value * refVal);
            /*** Alloc Related Methods ***/
            void addAlloc(Value *v, Type * ty);
            // void addAlloc(Value * v, Type * memType, Type * structType, long memberNum);
            bool AllocExists(Value *v, Type *ty);
            void setAllocList(BasicBlockList);
            // void incrementAllocatedRefCount(Value *v, Value * refVal);
            // void decrementAllocatedRefCount(Value *v, Value * refVal);
            /*** Live Variable Methods ***/
            // void addLiveVariable(Value * v, Type * memType, Type * structType, long memberNum);
            void setLiveVariables(LiveVariableList);
            void addLiveVariable(Value * v);
            void LiveVariableExists(Value * v);
            void incrementRefCount(Value * v);
            void decrementRefCount(Value * v);
            /*** Utilities ***/
            BasicBlockWorkList getWorkList(int mode) const;
            LiveVariableList getLiveVariables() const;
    };
    class BasicBlockManager {
        private:
            map<BasicBlock *,BasicBlockStat> BBMap;
            bool exists(BasicBlock *B);
            BasicBlockStat * get(BasicBlock *B);
            BasicBlockList intersectList(BasicBlockList src, BasicBlockList tgt);
            LiveVariableList intersectLiveVariables(LiveVariableList src, LiveVariableList tgt);
        public:
            void CollectInInfo(BasicBlock *B, bool isEntryPoint);
            void add(BasicBlock * B, Value *v, int mode);
            void add(BasicBlock * B, Value *v, Type * memTy, int mode);
            // void incrementRefCount(BasicBlock *B, Value *v, Value * refVal, int mode);
            // void decrementRefCount(BasicBlock *B, Value *v, Value * refVal, int mode);
            void copy(BasicBlock *src, BasicBlock *tgt);
            void intersect(BasicBlock *src, BasicBlock *tgt);
            BasicBlockList getBasicBlockFreeList(BasicBlock *src);
            BasicBlockList getBasicBlockAllocList(BasicBlock *src);
            void addLiveVariable(BasicBlock *B, Value *val);
            LiveVariableList getLiveVariables(BasicBlock *B);
    };
}
