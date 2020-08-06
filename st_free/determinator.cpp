#include "include/determinator.hpp"

#include "include/support_funcs.hpp"

using namespace ST_free;

const vector<string> alloc_funcs = {
    "malloc",        "calloc",           "kzalloc",
    "kmalloc",       "zalloc",           "vmalloc",
    "kcalloc",       "vzalloc",          "kzalloc_node",
    "kmalloc_array", "kmem_cache_alloc", "kmem_cache_alloc_node",
    "memdup",        "kmemdup",          "kstrdup"};
const vector<string> free_funcs = {"free", "kfree", "kzfree", "vfree",
                                   "kvfree"};
const vector<string> err_funcs = {"IS_ERR"};

namespace ST_free {
bool isAllocFunction(llvm::Function *F) {
  if (F && F->hasName()) {
    return findFunctionName(F->getName(), alloc_funcs);
  }
  return false;
}

bool isFreeFunction(llvm::Function *F) {
  if (F && F->hasName()) {
    string name = F->getName();
    return findFunctionName(F->getName(), free_funcs);
  }
  return false;
}

bool isIsErrFunction(llvm::Function *F) {
  if (F && F->hasName()) {
    return findFunctionName(F->getName(), err_funcs);
  }
  return false;
}

bool findFunctionName(string name, vector<string> func_list) {
  auto itr = find_if(func_list.begin(), func_list.end(), [name](string str) {
    size_t ind = name.find(".");
    string tgt = ind != string::npos ? name.substr(0, ind) : name;
    return str.compare(tgt) == 0;
  });

  if (itr != func_list.end()) return true;
  return false;
}
}  // namespace ST_free
