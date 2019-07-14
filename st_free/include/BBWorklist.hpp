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
    using Aliases = map<Value *, Value *>;
    class BasicBlockInformation {
        private:
            /*** BasicBlock Lists ***/
            BasicBlockWorkList freeList;
            BasicBlockWorkList allocList;
            BasicBlockWorkList correctlyFreed;
            LiveVariableList liveVariables;
            Aliases aliasMap;
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
            /*** CorrectlyFreed ***/
            void addCorrectlyFreedValue(Value * V, Type * T, long mem);
            bool CorrectlyFreedValueExists(Value * V, Type * T, long mem);
            BasicBlockWorkList getCorrectlyFreedValues() const;
            /*** Utilities ***/
            BasicBlockWorkList getWorkList(int mode) const;
            LiveVariableList getLiveVariables() const;
            bool aliasExists(Value *);
            Value* getAlias(Value *);
            void setAlias(Value* src, Value* dest); //dest <- src
    };
    class BasicBlockManager {
        private:
            map<BasicBlock *,BasicBlockInformation> BBMap;
            BasicBlockList intersectList(BasicBlockList src, BasicBlockList tgt);
            LiveVariableList intersectLiveVariables(LiveVariableList src, LiveVariableList tgt);
            bool exists(BasicBlock *B);
        public:
            /*** getter ***/
            void set(BasicBlock *B);
            BasicBlockInformation * get(BasicBlock *B);
            BasicBlockList getBasicBlockFreeList(BasicBlock *src);
            BasicBlockList getBasicBlockAllocList(BasicBlock *src);
            LiveVariableList getLiveVariables(BasicBlock *B);
            /*** Mediator ***/
            void CollectInInfo(BasicBlock *B, bool isEntryPoint);
            void copy(BasicBlock *src, BasicBlock *tgt);
            void copyCorrectlyFreed(BasicBlock *src, BasicBlock *tgt);
            // void copyCorrectlyFreedToPrev(BasicBlock *src);
            void updateSuccessorBlock(BasicBlock *src);
            void intersect(BasicBlock *src, BasicBlock *tgt);
            bool isPredBlockCorrectlyBranched(BasicBlock *B);
    };
}
