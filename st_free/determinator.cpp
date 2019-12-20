#include "determinator.hpp"
#include "support_funcs.hpp"

using namespace ST_free;

const vector<string> alloc_funcs = {"malloc", "calloc", "kzalloc", "kmalloc", "zalloc", "vmalloc", "kcalloc", "vzalloc", "kzalloc_node", "kmalloc_array", "kmem_cache_alloc", "kmem_cache_alloc_node"};
const vector<string> free_funcs = {"free", "kfree", "kzfree", "vfree", "kvfree"};
const vector<string> err_funcs = {"IS_ERR"};

namespace ST_free {
    bool isAllocFunction(Function *F) {
        if (F && F->hasName()) {
            string name = F->getName();
            auto itr = find_if(alloc_funcs.begin(), alloc_funcs.end(),
                    [name](string str) {
                        size_t ind = name.find(".");
                        string tgt = ind != string::npos ? name.substr(0, ind) : name; 
                        return str.compare(tgt) == 0;
                    }
                );
            if(itr != alloc_funcs.end())
                return true;
        }
        return false;
    }

    bool isFreeFunction(Function *F) {
        if (F && F->hasName()) {
            string name = F->getName();
            auto itr = find_if(free_funcs.begin(), free_funcs.end(),
                    [name](string str) {
                        size_t ind = name.find(".");
                        string tgt = ind != string::npos ? name.substr(0, ind) : name; 
                        return str.compare(tgt) == 0;
                    }
                );
            if(itr != free_funcs.end())
                return true;
        }
        return false;
    }

    bool isIsErrFunction(Function *F) {
        if (F && F->hasName()) {
            string name = F->getName();
            auto itr = find_if(err_funcs.begin(), err_funcs.end(),
                    [name](string str) {
                        size_t ind = name.find(".");
                        string tgt = ind != string::npos ? name.substr(0, ind) : name; 
                        return str.compare(tgt) == 0;
                    }
                );
            if(itr != err_funcs.end())
                return true;
        }
        return false;
    }
}
