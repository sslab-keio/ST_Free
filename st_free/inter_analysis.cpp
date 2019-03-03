#include "inter_analysis.hpp"

namespace ST_free{
    bool Analyzer::isInMap(Function *F){
        return false;
    }

    bool Analyzer::isInMap(Function *F, int arg_no){
        return false;
    }


    vector<vector<Value *>> * Analyzer::getFunctionValueList(Function * F){
        if(isInMap(F)){
            return &(func_map[F]);
        }
        return NULL;
    }

    vector<Value *> * Analyzer::getOperandValues(Function *F, int arg_no){
        if(isInMap(F)){
            return &(func_map[F][arg_no]);
        }
        return NULL;
    }

    void Analyzer::addValue(Function *F, Value *V, int arg_no){
        if(isInMap(F)){
            func_map[F][arg_no].push_back(V);
        }
        return;
    }

    void Analyzer::setFunction(Function *F){
        if(!isInMap(F)){
            func_map[F] = vector<vector<Value *>>(F->arg_size());
        }
        return;
    }
}
