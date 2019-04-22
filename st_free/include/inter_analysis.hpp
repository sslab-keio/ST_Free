#include "ST_free.hpp"
#pragma once

namespace ST_free{
    struct FuncElement {
        private:
            int argNum;
            int stat;
            Value *arg;
        public:
            FuncElement();
            bool isArgAllocated();
            bool isArgFreed();
            int getStat();
            void setStat(int);
    };

    class FuncIdentifier{
        private:
            map<Function *, vector<struct FuncElement>> func_map;
            // map<Function *, vector<int>> func_map;
        public:
            bool exists(Function *);
            bool exists(Function *, int);
            vector<struct FuncElement> getArgStatList(Function *);
            // vector<int> * getArgStatList(Function *);
            vector<struct FuncElement>::iterator itr_begin(Function *);
            vector<struct FuncElement>::iterator itr_end(Function *);
            int getArgStat(Function *, int);
            void initFuncStat(Function *);
            void setFuncArgStat(Function *, int, int);
            size_t getArgSize(Function *F);
            bool isArgAllocated(Function *, int);
            bool isArgFreed(Function *, int);
    };
}
