#include "ST_free.hpp"
#pragma once

namespace ST_free {
    using BasicBlockList = vector<Value *>;
    class BasicBlockWorkList {
        private:
            BasicBlockList MarkedValues;
        public:
            BasicBlockWorkList();
            BasicBlockWorkList(const BasicBlockList);
            void add(Value * v);
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
            bool FreeExists(Value *v);
            void setFreeList(BasicBlockList);
            void addAlloc(Value *v);
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
            void copy(BasicBlock *src, BasicBlock *tgt);
            void intersect(BasicBlock *src, BasicBlock *tgt);
            BasicBlockList getBasicBlockFreeList(BasicBlock *src);
            BasicBlockList getBasicBlockAllocList(BasicBlock *src);
    };
}
