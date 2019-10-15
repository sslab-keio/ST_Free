#include "ST_free.hpp"
#pragma once

namespace ST_free{
    LoadInst * find_load(Instruction *);
    Value * getLoadeeValue(Value *);
    Type * get_type(Value *);
    Type * get_type(Type *);
    void generateWarning(Instruction *, std::string);
    void generateWarning(string warn);
    void generateWarning(Instruction * Inst, Value *val);
    void generateError(Instruction *, std::string);
    Value * getArgAlloca(Value *arg);
    long getValueIndices(GetElementPtrInst * inst);
    static LoadInst * find_load_recursively(Instruction *val, int TTL);
    User * getFirstUser(Value *);
    GetElementPtrInst *getRootGEle(GetElementPtrInst *GEle);
}
