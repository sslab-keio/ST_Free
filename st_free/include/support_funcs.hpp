#include "ST_free.hpp"
#pragma once

#ifdef LOG_ALL_ENABLED
#define STFREE_LOG(Inst, message) ST_free::generateWarning(Inst, message, true)
#else
#define STFREE_LOG(Inst, message) ST_free::generateWarning(Inst, message)
#endif

#define STFREE_LOG_ON(Inst, message) ST_free::generateWarning(Inst, message, true)

namespace ST_free {
llvm::LoadInst *find_load(llvm::Instruction *);
llvm::Value *getLoadeeValue(llvm::Value *);
llvm::Type *get_type(llvm::Value *);
llvm::Type *get_type(llvm::Type *);
llvm::Type *decode_array_type(llvm::Type *);
void generateWarning(llvm::Instruction *, std::string, bool print = false);
void generateWarning(std::string warn);
void generateWarning(llvm::Instruction *Inst, llvm::Value *val);
void generateError(llvm::Instruction *, std::string);
std::string parseErrorMessage(llvm::StructType *parent, long index);
llvm::Value *getArgAlloca(llvm::Value *arg);
static llvm::LoadInst *find_load_recursively(llvm::Instruction *val, int TTL);
}  // namespace ST_free
