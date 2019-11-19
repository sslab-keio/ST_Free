#include "determinator.hpp"
#include "support_funcs.hpp"

using namespace ST_free;

const vector<string> alloc_funcs = {"malloc", "calloc", "kzalloc", "kmalloc", "zalloc", "vmalloc", "kcalloc", "vzalloc"};
const vector<string> free_funcs = {"free", "kfree", "kzfree", "vfree", "kvfree"};

namespace ST_free {
    bool isAllocFunction(Function *F) {
        if (F && F->hasName()) {
            string name = F->getName();
            auto itr = find(alloc_funcs.begin(), alloc_funcs.end(), name);
            if(itr != alloc_funcs.end())
                return true;
        }
        return false;
    }

    bool isFreeFunction(Function *F) {
        if (F->hasName()) {
            string name = F->getName();
            auto itr = find(free_funcs.begin(), free_funcs.end(), name);
            if(itr != free_funcs.end())
                return true;
        }
        return false;
    }
}
