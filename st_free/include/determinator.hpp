#include "ST_free.hpp"
#pragma once

namespace ST_free {
    bool isAllocFunction(Function *);
    bool isFreeFunction(Function *);
    bool isStructEleAlloc(Instruction *);
    bool isStructEleFree(Instruction *);
    bool isStructFree(Instruction *);
    bool isOptimizedStructFree(Instruction *I);
    Type* getOptimizedStructFree(Instruction *I);
    Value* getStructFreedValue(Instruction * val);
    bool isHeapValue(Value *);
    bool isFuncPointer(Type * t);
    GetElementPtrInst* getAllocStructEleInfo(Instruction *);
    GetElementPtrInst* getFreeStructEleInfo(Instruction *);
    GetElementPtrInst*  getStoredStructEle(StoreInst * SI);
    GetElementPtrInst*  getStoredStruct(StoreInst * SI);
    Type * getStructType(Instruction * val);
    Value * getFreedValue(Instruction * val);
    Value * getAllocatedValue(Instruction *I) ;
}
