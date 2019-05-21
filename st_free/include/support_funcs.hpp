#include "ST_free.hpp"
#pragma once

namespace ST_free{
    LoadInst * find_load(Instruction *);
    Value * getLoadeeValue(Value *);
    Type * get_type(Value *);
    void generateWarning(Instruction *, std::string);
    Value * getArgAlloca(Value *arg);
    long getValueIndices(GetElementPtrInst * inst);
}
