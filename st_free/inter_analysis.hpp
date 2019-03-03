#include "ST_free.hpp"
#pragma once

namespace ST_free{
    class Analyzer{
        private:
            map<Function *, vector<vector<Value *>>> func_map;
        public:
            bool isInMap(Function *);
            bool isInMap(Function *, int);
            vector<vector<Value *>> * getFunctionValueList(Function *);
            vector<Value *> * getOperandValues(Function *, int);
            void addValue(Function *, Value *, int);
            void setFunction(Function *);
    };
}
