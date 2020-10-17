#include "include/determinator.hpp"

#include "include/support_funcs.hpp"

using namespace ST_free;

const std::vector<std::string> alloc_funcs = {
    "malloc",        "calloc",           "kzalloc",
    "kmalloc",       "zalloc",           "vmalloc",
    "kcalloc",       "vzalloc",          "kzalloc_node",
    "kmalloc_array", "kmem_cache_alloc", "kmem_cache_alloc_node",
    "memdup",        "kmemdup",          "kstrdup"};
const std::vector<std::string> free_funcs = {"free", "kfree", "kzfree", "vfree",
                                   "kvfree"};

const std::vector<std::string> err_funcs = {"IS_ERR"};

const std::vector<std::string> specialized_free_funcs = {"put_device", "kobject_put"};

const std::vector<std::string> data_struct_node = {"struct.list_head", "struct.rb_node"};

namespace ST_free {
bool isAllocFunction(llvm::Function *F) {
  if (F && F->hasName()) {
    return findFunctionName(F->getName(), alloc_funcs);
  }
  return false;
}

bool isFreeFunction(llvm::Function *F) {
  if (F && F->hasName()) {
    std::string name = F->getName();
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

bool isSpecializedFreeFunction(llvm::Function* F) {
  if (F && F->hasName()) {
    return findFunctionName(F->getName(), specialized_free_funcs);
  }
  return false;
}

bool isStructDataStructNode(llvm::StructType *StTy) {
  if (StTy && StTy->hasName()) {
		std::string name = StTy->getName();
		auto itr = find_if(
				data_struct_node.begin(),
				data_struct_node.end(),
				[name](std::string str) {
			// size_t ind = name.find_last_of(".");
			// std::string tgt = ind != std::string::npos ? name.substr(0, ind) : name;
			// llvm::outs() << tgt << "\n";
			return str.compare(name) == 0;
		});

		if (itr != data_struct_node.end()) return true;
  }
  return false;
}

bool findFunctionName(std::string name, std::vector<std::string> func_list) {
  auto itr = find_if(func_list.begin(), func_list.end(), [name](std::string str) {
    size_t ind = name.find(".");
    std::string tgt = ind != std::string::npos ? name.substr(0, ind) : name;
    return str.compare(tgt) == 0;
  });

  if (itr != func_list.end()) return true;
  return false;
}
}  // namespace ST_free
