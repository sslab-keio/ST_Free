#include "inter_analysis.hpp"

namespace ST_free{
    bool FuncIdentifier::isInMap(Function *F){
        if(func_map.find(F) != func_map.end())
            return true;
        return false;
    }

    bool FuncIdentifier::isInMap(Function *F, int arg_no){
         if(func_map.find(F) != func_map.end() && arg_no < F->arg_size())
            return true;
        return false;
    }

    vector<int> * FuncIdentifier::getStatusList(Function * F){
        if(this->isInMap(F)){
            return &(func_map[F]);
        }
        return NULL;
    }

    int FuncIdentifier::getStatus(Function *F, int arg_no){
        if(this->isInMap(F, arg_no)){
            return func_map[F][arg_no];
        }
        return NO_ALLOC;
    }

    void FuncIdentifier::setFunction(Function *F){
        if(!this->isInMap(F)){
            func_map[F] = vector<int>(F->arg_size(), NO_ALLOC);
        }
        return;
    }

    void FuncIdentifier::setFunctionStatus(Function *F, int arg_no, int status){
        if(this->isInMap(F, arg_no)){
            func_map[F][arg_no] = status;
        }
        return;
    }
}
