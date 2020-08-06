#include "ST_free.hpp"
#pragma once
namespace ST_free {
class StatusList {
 private:
  std::map<llvm::Type *, std::map<llvm::Value *, std::vector<int>>> st_tab;

 public:
  int getStat(llvm::Type *, llvm::Value *, uint64_t);
  void setStat(llvm::Type *, llvm::Value *, uint64_t, int);
  bool exists(llvm::Type *, llvm::Value *, uint64_t);
  bool exists(llvm::Type *, llvm::Value *);
  void setList(llvm::Type *, llvm::Value *);
  std::vector<int> *getList(llvm::Type *, llvm::Value *);
  size_t getSize(llvm::Type *, llvm::Value *);
};
}  // namespace ST_free
