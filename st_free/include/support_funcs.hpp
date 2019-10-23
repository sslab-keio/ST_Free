#include "ST_free.hpp"
#pragma once

namespace ST_free{
    LoadInst * find_load(Instruction *);
    Value * getLoadeeValue(Value *);
    Type * get_type(Value *);
    Type * get_type(Type *);
    void generateWarning(Instruction *, std::string, bool print = false);
    void generateWarning(string warn);
    void generateWarning(Instruction * Inst, Value *val);
    void generateError(Instruction *, std::string);
    string parseErrorMessage(StructType* parent, long index);
    Value * getArgAlloca(Value *arg);
    static LoadInst * find_load_recursively(Instruction *val, int TTL);
}
