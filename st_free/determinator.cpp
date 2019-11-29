#include "determinator.hpp"
#include "support_funcs.hpp"

using namespace ST_free;

const vector<string> alloc_funcs = {"malloc", "calloc", "kzalloc", "kmalloc", "zalloc", "vmalloc", "kcalloc", "vzalloc"};
const vector<string> free_funcs = {"free", "kfree", "kzfree", "vfree", "kvfree"};

namespace ST_free {
    bool isAllocFunction(Function *F) {
        if (F && F->hasName()) {
            string name = F->getName();
            auto itr = find_if(alloc_funcs.begin(), alloc_funcs.end(),
                    [name](string str) {
                        size_t ind = str.find('.');
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
                        size_t ind = str.find('.');
                        string tgt = ind != string::npos ? name.substr(0, ind) : name; 
                        return str.compare(tgt) == 0;
                    }
                );
            if(itr != free_funcs.end())
                return true;
        }
        return false;
    }
}
