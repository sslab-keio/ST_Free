#pragma once
#include "ST_free.hpp"
#include "BBWorklist.hpp"
#include "argList.hpp"

namespace ST_free{
    struct FunctionInformation {
        private:
            Function *F;
            int stat;
            ArgList args;
            vector<BasicBlock *> endPoint;
            vector<Value *> freedStruct;
            BasicBlockManager BBManage;
            int getStat();
            void setStat(int);
        public:
            FunctionInformation();
            FunctionInformation(Function *F);
            void addEndPoint(BasicBlock *B);
            void addFreeValue(BasicBlock *B, Value *V);
            void addAllocValue(BasicBlock *B, Value *V);
            void addFreedStruct(Value *V);
            vector<Value *> getFreedStruct(Value *V) const;
            bool isUnanalyzed();
            bool isAnalyzed();
            bool isInProgress();
            void setAnalyzed();
            void setInProgress();
            Function & getFunction();
            void BBCollectInfo(BasicBlock& B, bool isEntryPoint);
    };

    class FunctionManager {
        private:
            map<Function *, FunctionInformation *> func_map;
        public:
            bool exists(Function *);
            FunctionInformation * getElement(Function *F);
    };
}
