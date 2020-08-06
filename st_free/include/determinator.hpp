#include "ST_free.hpp"
#pragma once

namespace ST_free {
bool isAllocFunction(llvm::Function *);
bool isFreeFunction(llvm::Function *);
bool isIsErrFunction(llvm::Function *);
bool findFunctionName(std::string name, std::vector<std::string> func_list);
}  // namespace ST_free
