#include "inter_analysis.hpp"

namespace ST_free{
    bool FuncIdentifier::exists(Function *F){
        if(func_map.find(F) != func_map.end())
            return true;
        return false;
    }

    bool FuncIdentifier::exists(Function *F, int arg_no){
         if(func_map.find(F) != func_map.end() && arg_no < F->arg_size())
            return true;
        return false;
    }

    // vector<int> * FuncIdentifier::getArgStatList(Function * F){
    //     if(this->exists(F)){
    //         return &(func_map[F]);
    //     }
    //     return NULL;
    // }

    vector<struct FuncElement> * FuncIdentifier::getArgStatList(Function * F){
        if(this->exists(F)){
            return &(func_map[F]);
        }
        return NULL;
    }

    int FuncIdentifier::getArgStat(Function *F, int arg_no){
        if(this->exists(F, arg_no)){
            return func_map[F][arg_no].getStat();
        }
        return NO_ALLOC;
    }

    void FuncIdentifier::initFuncStat(Function *F){
        if(!this->exists(F)){
            func_map[F] = vector<struct FuncElement>(F->arg_size(), FuncElement());
        }
        return;
    }

    void FuncIdentifier::setFuncArgStat(Function *F, int arg_no, int status){
        if(this->exists(F, arg_no)){
            func_map[F][arg_no].setStat(status);
        }
        return;
    }

    size_t FuncIdentifier::getArgSize(Function *F){
        if(this->exists(F))
            return func_map[F].size();
        return 0;
    }

    vector<struct FuncElement>::iterator FuncIdentifier::itr_begin(Function *F){
        // if(this->exists(F))
            return func_map[F].begin();
        // return NULL;
    }

    vector<struct FuncElement>::iterator FuncIdentifier::itr_end(Function *F){
        // if(this->exists(F))
            return func_map[F].end();
        // return NULL;
    }

    bool FuncIdentifier::isArgAllocated(Function *F, int argNum){
        int stat = getArgStat(F, argNum);
        return (stat == ALLOCATED) ? true : false;
    }

    bool FuncIdentifier::isArgFreed(Function *F, int argNum){
        int stat = getArgStat(F, argNum);
        return (stat == FREED) ? true : false;
    }

    FuncElement::FuncElement(){
        stat = NO_ALLOC;
        argNum = 0;
        arg = NULL;
    }

    bool FuncElement::isArgAllocated(){
        return (this->stat == ALLOCATED) ? true : false;
    }

    bool FuncElement::isArgFreed(){
        return (this->stat == FREED) ? true : false;
    }

    int FuncElement::getStat(){
        return this->stat;
    }

    void FuncElement::setStat(int stat){
        this->stat = stat;
    }
}
