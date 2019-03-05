#include "argList.hpp"

namespace ST_free{
    void ArgList::setArg(uint64_t arg_no, Value *V){
        if(arg_no < arg_list.size()){
             arg_list[arg_no] = V;
        }
        return;
    }

    Value * ArgList::getArg(uint64_t arg_no){
        if(arg_no < arg_list.size()){
            return arg_list[arg_no];
        }
        return NULL;
    }

    void ArgList::setArgs(Function &F){
        int i = 0;
        for(auto args = F.arg_begin(); args != F.arg_end(); args++, i++){
            Value * v = getArgAlloca(args);
            if(v != NULL){
                this->setArg(i, v);
            }
        }
        return;
    }
}
