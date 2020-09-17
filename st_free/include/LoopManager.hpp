#include "ST_free.hpp"
#pragma once

namespace ST_free {
class LoopManager {
 public:
  LoopManager(){};
  void add(llvm::Function *F, llvm::LoopInfo li);
  bool exists(llvm::Function *F);
  llvm::LoopInfo *get(llvm::Function *F);

  bool IsInLoop(llvm::Function *F, llvm::BasicBlock *B);
  bool IsHeaderBlock(llvm::Function *F, llvm::BasicBlock *B);
  bool IsPreheaderBlock(llvm::Function *F, llvm::BasicBlock *B);
  llvm::Loop *getLoop(llvm::Function *F, llvm::BasicBlock *B);

 private:
  std::map<llvm::Function *, llvm::LoopInfo> loopinfo;
};
}  // namespace ST_free
