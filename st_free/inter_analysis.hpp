#include "ST_free.hpp"
#pragma once

namespace ST_free{
    class FuncIdentifier{
        private:
            map<Function *, vector<int>> func_map;
        public:
            bool isInMap(Function *);
            bool isInMap(Function *, int);
            vector<int> * getStatusList(Function *);
            int getStatus(Function *, int);
            void setFunction(Function *);
            void setFunctionStatus(Function *, int, int);
    };
}
