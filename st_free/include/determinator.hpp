#include "ST_free.hpp"
#pragma once

namespace ST_free {
    bool isAllocFunction(Function *);
    bool isFreeFunction(Function *);
    bool isIsErrFunction(Function *);
    bool findFunctionName(string name, vector<string> func_list);
}
