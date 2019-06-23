#include "ST_free.hpp"
#include "ValueInformation.hpp"
#pragma once

namespace ST_free {
    using BasicBlockList = vector<uniqueKey>;
    class BasicBlockWorkList {
        private:
            BasicBlockList MarkedValues;
        public:
            BasicBlockWorkList();
            BasicBlockWorkList(const BasicBlockList);
            void add(Value * v, Type * t, long mem);
            bool exists(Value * v, Type *t, long mem);
            BasicBlockList getList() const;
            void setList(BasicBlockList);
    };

    using LiveVariableList = vector<Value *>;
    class BasicBlockInformation {
        private:
            /*** BasicBlock Lists ***/
            BasicBlockWorkList freeList;
            BasicBlockWorkList allocList;
            BasicBlockWorkList correctlyFreed;
            LiveVariableList liveVariables;
            /*** BasicBlock Status ***/
            bool correctlyBranched;
            bool predCorrectlyBranched;
            bool loopBlock;
        public:
            BasicBlockInformation();
            BasicBlockInformation(const BasicBlockInformation &);
            /*** Free Related Methods ***/
            void addFree(Value * v, Type * ty, long mem);
            bool FreeExists(Value *v, Type * ty, long mem);
            void setFreeList(BasicBlockList);
            /*** Alloc Related Methods ***/
            void addAlloc(Value *v, Type * ty, long mem);
            bool AllocExists(Value *v, Type *ty, long mem);
            void setAllocList(BasicBlockList);
            /*** Live Variable Methods ***/
            void setLiveVariables(LiveVariableList);
            void addLiveVariable(Value * v);
            bool LiveVariableExists(Value * v);
            void incrementRefCount(Value * v);
            void decrementRefCount(Value * v);
            /*** Correct Branch Freed Methods ***/
            void setCorrectlyBranched();
            bool isCorrectlyBranched();
            void setLoopBlock();
            bool isLoopBlock();
            void addCorrectlyFreedValue(Value * V, Type * T, long mem);
            bool CorrectlyFreedValueExists(Value * V, Type * T, long mem);
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
            void add(BasicBlock * B, Value *v, Type * memTy, long mem, int mode);
            void copy(BasicBlock *src, BasicBlock *tgt);
            void intersect(BasicBlock *src, BasicBlock *tgt);
            BasicBlockList getBasicBlockFreeList(BasicBlock *src);
            BasicBlockList getBasicBlockAllocList(BasicBlock *src);
            void addLiveVariable(BasicBlock *B, Value *val);
            LiveVariableList getLiveVariables(BasicBlock *B);
            bool existsInFreedList(BasicBlock *B, Value *val, Type *ty, long mem);
            bool existsInAllocatedList(BasicBlock *B, Value *val, Type *ty, long mem);
            bool existsInLiveVariableList(BasicBlock * B, Value *val);
            void setCorrectlyBranched(BasicBlock * B);
            bool isCorrectlyBranched(BasicBlock * B);
            void setLoopBlock(BasicBlock *B);
            bool isLoopBlock(BasicBlock *B);
            bool isPredBlockCorrectlyBranched(BasicBlock *B);
            void addCorrectlyFreedValue(BasicBlock *, Value *, Type *, long mem);
            bool correctlyFreedValueExists(BasicBlock *, Value *, Type *, long mem);
    };
}
