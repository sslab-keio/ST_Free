#include "ST_free.hpp"
#pragma once

namespace ST_free{
    class FuncIdentifier{
        private:
            map<Function *, vector<int>> func_map;
        public:
            bool exists(Function *);
            bool exists(Function *, int);
            vector<int> * getArgStatList(Function *);
            int getArgStat(Function *, int);
            void initFuncStat(Function *);
            void setFuncArgStat(Function *, int, int);
            size_t getArgSize(Function *F);
            bool isArgAllocated(Function *, int);
            bool isArgFreed(Function *, int);
    }
