#include "ST_free.hpp"
#include "ValueInformation.hpp"
#pragma once

namespace ST_free {
    using uniqueKey = pair<Value *, Type *>;
    using BasicBlockList = vector<uniqueKey>;
    // using BasicBlockList = vector<ValueInformation>;
    class BasicBlockWorkList {
        private:
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
    class BasicBlockInformation {
        private:
            BasicBlockWorkList freeList;
            BasicBlockWorkList allocList;
            BasicBlockWorkList correctlyFreed;
            LiveVariableList liveVariables;
            bool correctlyBranched;
            bool predCorrectlyBranched;
        public:
            BasicBlockInformation();
            BasicBlockInformation(const BasicBlockInformation &);
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
            bool LiveVariableExists(Value * v);
            void incrementRefCount(Value * v);
            void decrementRefCount(Value * v);
            /*** Correct Branch Freed Methods ***/
            void setCorrectlyBranched();
            bool isCorrectlyBranched();
            void addCorrectlyFreedValue(Value * V, Type * T);
            bool CorrectlyFreedValueExists(Value * V, Type * T);
            BasicBlockWorkList getCorrectlyFreedValues() const;
            /*** Utilities ***/
            BasicBlockWorkList getWorkList(int mode) const;
            LiveVariableList getLiveVariables() const;
    };
    class BasicBlockManager {
        private:
            map<BasicBlock *,BasicBlockInformation> BBMap;
            bool exists(BasicBlock *B);
            BasicBlockInformation * get(BasicBlock *B);
            BasicBlockList intersectList(BasicBlockList src, BasicBlockList tgt);
            LiveVariableList intersectLiveVariables(LiveVariableList src, LiveVariableList tgt);
        public:
            void CollectInInfo(BasicBlock *B, bool isEntryPoint);
            void add(BasicBlock * B, Value *v, int mode);
            void add(BasicBlock * B, Value *v, Type * memTy, int mode);
            void copy(BasicBlock *src, BasicBlock *tgt);
            void intersect(BasicBlock *src, BasicBlock *tgt);
            BasicBlockList getBasicBlockFreeList(BasicBlock *src);
            BasicBlockList getBasicBlockAllocList(BasicBlock *src);
            void addLiveVariable(BasicBlock *B, Value *val);
            LiveVariableList getLiveVariables(BasicBlock *B);
            void existsInFreedList(BasicBlock *B, Value *val, Type *ty);
            void existsInAllocatedList(BasicBlock *B, Value *val, Type *ty);
            bool existsInLiveVariableList(BasicBlock * B, Value *val);
            void setCorrectlyBranched(BasicBlock * B);
            bool isCorrectlyBranched(BasicBlock * B);
            bool isPredBlockCorrectlyBranched(BasicBlock *B);
            void addCorrectlyFreedValue(BasicBlock *, Value *, Type *);
            bool correctlyFreedValueExists(BasicBlock *, Value *, Type *);
    };
}
