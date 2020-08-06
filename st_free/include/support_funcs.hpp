#include "ST_free.hpp"
#pragma once

namespace ST_free {
llvm::LoadInst *find_load(llvm::Instruction *);
llvm::Value *getLoadeeValue(llvm::Value *);
llvm::Type *get_type(llvm::Value *);
llvm::Type *get_type(llvm::Type *);
void generateWarning(llvm::Instruction *, std::string, bool print = false);
void generateWarning(std::string warn);
void generateWarning(llvm::Instruction *Inst, llvm::Value *val);
void generateError(llvm::Instruction *, std::string);
std::string parseErrorMessage(llvm::StructType *parent, long index);
llvm::Value *getArgAlloca(llvm::Value *arg);
static llvm::LoadInst *find_load_recursively(llvm::Instruction *val, int TTL);
}  // namespace ST_free
