#include "ST_free.hpp"
#pragma once

namespace ST_free{
    bool isAllocFunction(string);
    // bool isAllocFunction(Function *);
    bool isFreeFunction(string);
    // bool isFreeFunction(Function *);
    bool isStructEleAlloc(Instruction *);
    bool isStructEleFree(Instruction *);
    bool isStructFree(Instruction *);
    bool isHeapValue(Value *);
    GetElementPtrInst * getAllocStructEleInfo(Instruction *);
    GetElementPtrInst* getFreeStructEleInfo(Instruction *);
}
