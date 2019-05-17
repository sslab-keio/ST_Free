#include "inter_analysis.hpp"

namespace ST_free{
    bool FunctionManager::exists(Function *F) {
        if(func_map.find(F) != func_map.end())
            return true;
        return false;
    }

    FuncElement* FunctionManager::getElement(Function *F){
        if(!this->exists(F))
            func_map[F] = new struct FuncElement(F);
        return func_map[F];
    }

    FuncElement::FuncElement(Function *Func){
        if (Func != NULL){
            F = Func;
            args = ArgList(Func->arg_size());
            args.setArgs(Func);
            stat = UNANALYZED;
        }
    }

    FuncElement::FuncElement(){
        stat = UNANALYZED;
    }

    Function & FuncElement::getFunction(){
        return (Function &)(* this->F);
    }

    int FuncElement::getStat(){
        return this->stat;
    }

    void FuncElement::setStat(int stat){
        this->stat = stat;
    }

    void FuncElement::addEndPoint(BasicBlock *B){
        endPoint.push_back(B);
    }

    void FuncElement::addFreeValue(BasicBlock *B, Value *V) {
        BBManage.add(B, V, FREED);
    }

    void FuncElement::addAllocValue(BasicBlock *B, Value *V) {
        BBManage.add(B, V, ALLOCATED);
    }

    bool FuncElement::isUnanalyzed(){
        return getStat() == UNANALYZED ? true : false;
    }
    bool FuncElement::isInProgress(){
        return getStat() == IN_PROGRESS ? true : false;
    }
    bool FuncElement::isAnalyzed(){
        return getStat() == ANALYZED ? true : false;
    }
    void FuncElement::setAnalyzed(){
        setStat(ANALYZED);
    }

    void FuncElement::setInProgress(){
        setStat(IN_PROGRESS);
    }
    void FuncElement::BBCollectInfo(BasicBlock *B, bool isEntryPoint){
        BBManage.CollectInInfo(B, isEntryPoint);
    }
}
