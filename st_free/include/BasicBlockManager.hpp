#pragma once
#include "ST_free.hpp"
#include "ValueInformation.hpp"
#include "UniqueKeyManager.hpp"

namespace ST_free {
    using BasicBlockList = vector<const UniqueKey *>;
    class BasicBlockWorkList {
        public:
            BasicBlockWorkList();
            BasicBlockWorkList(const BasicBlockList);
            // void add(Value * v, Type * t, long mem);
            void add(const UniqueKey *UK);
            bool exists(const UniqueKey *UK);
            // bool exists(Value * v, Type *t, long mem);
            BasicBlockList getList() const;
            void setList(BasicBlockList);
        private:
            BasicBlockList MarkedValues;
    };

    using LiveVariableList = vector<Value *>;
    using Aliases = map<Value *, Value *>;
    class BasicBlockInformation {
        public:
            BasicBlockInformation();
            BasicBlockInformation(const BasicBlockInformation &);
            /*** Free Related Methods ***/
            // void addFree(Value * v, Type * ty, long mem);
            void addFree(const UniqueKey *UK);
            // bool FreeExists(Value *v, Type * ty, long mem);
            bool FreeExists(const UniqueKey *UK);
            void setFreeList(BasicBlockList);
            /*** Alloc Related Methods ***/
            // void addAlloc(Value *v, Type * ty, long mem);
            void addAlloc(const UniqueKey *UK);
            // bool AllocExists(Value *v, Type *ty, long mem);
            bool AllocExists(const UniqueKey *UK);
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
            // void addCorrectlyFreedValue(Value * V, Type * T, long mem);
            void addCorrectlyFreedValue(const UniqueKey *UK);
            // bool CorrectlyFreedValueExists(Value * V, Type * T, long mem);
            bool CorrectlyFreedValueExists(const UniqueKey *UK);
            BasicBlockWorkList getCorrectlyFreedValues() const;
            /*** Utilities ***/
            BasicBlockWorkList getWorkList(int mode) const;
            LiveVariableList getLiveVariables() const;
            bool aliasExists(Value *);
            Value* getAlias(Value *);
            void setAlias(Value* src, Value* dest); //dest <- src
            void addStoredCallValues(Value *v, CallInst *CI);
            vector<pair<Value *, CallInst *>> getStoredCallValues();
            bool isCallValues(Value *V);
            CallInst *getCallInstForVal(Value *V);
        private:
            /*** BasicBlock Lists ***/
            BasicBlockWorkList freeList;
            BasicBlockWorkList allocList;
            BasicBlockWorkList correctlyFreed;
            LiveVariableList liveVariables;
            Aliases aliasMap;
            vector<pair<Value *, CallInst *>> storedCallValues;
            /*** BasicBlock Status ***/
            bool correctlyBranched;
            bool predCorrectlyBranched;
            bool loopBlock;
    };
    class BasicBlockManager {
        public:
            /*** getter ***/
            void set(BasicBlock *B);
            BasicBlockInformation* get(BasicBlock *B);
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
            void unite(BasicBlock *src, BasicBlock *tgt);
            bool isPredBlockCorrectlyBranched(BasicBlock *B);
        private:
            map<BasicBlock *,BasicBlockInformation> BBMap;
            BasicBlockList intersectList(BasicBlockList src, BasicBlockList tgt);
            BasicBlockList uniteList(BasicBlockList src, BasicBlockList tgt);
            LiveVariableList intersectLiveVariables(LiveVariableList src, LiveVariableList tgt);
            bool exists(BasicBlock *B);
    };
}
