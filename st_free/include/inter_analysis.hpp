#pragma once
#include "ST_free.hpp"
#include "BBWorklist.hpp"
#include "argList.hpp"

namespace ST_free{
    struct FuncElement {
        private:
            Function *F;
            int stat;
            // Value *arg;
            ArgList args;
            vector<BasicBlock *> endPoint;
            BasicBlockManager BBManage;
            int getStat();
            void setStat(int);
        public:
            FuncElement();
            FuncElement(Function *F);
            void addEndPoint(BasicBlock *B);
            void addFreeValue(BasicBlock *B, Value *V);
            void addAllocValue(BasicBlock *B, Value *V);
            bool isUnanalyzed();
            bool isAnalyzed();
            bool isInProgress();
            void setAnalyzed();
            void setInProgress();
            Function & getFunction();
            void BBCollectInfo(BasicBlock *B, bool isEntryPoint);
    };

    class FunctionManager {
        private:
            map<Function *, struct FuncElement *> func_map;
        public:
            bool exists(Function *);
            struct FuncElement * getElement(Function *F);
    };
}
