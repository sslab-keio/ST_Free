#include "functionManager.hpp"

namespace ST_free{
    bool FunctionManager::exists(Function *F) {
        if(func_map.find(F) != func_map.end())
            return true;
        return false;
    }

    FunctionInformation* FunctionManager::getElement(Function *F){
        if(!this->exists(F))
            func_map[F] = new struct FunctionInformation(F);
        return func_map[F];
    }

    FunctionInformation::FunctionInformation(Function *Func){
        if (Func != NULL){
            F = Func;
            args = ArgList(Func->arg_size());
            args.setArgs(Func);
            stat = UNANALYZED;
        }
    }

    FunctionInformation::FunctionInformation(){
        stat = UNANALYZED;
    }

    Function & FunctionInformation::getFunction(){
        return (Function &)(* this->F);
    }

    int FunctionInformation::getStat(){
        return this->stat;
    }

    void FunctionInformation::setStat(int stat){
        this->stat = stat;
    }

    void FunctionInformation::addEndPoint(BasicBlock *B){
        endPoint.push_back(B);
    }

    void FunctionInformation::addFreeValue(BasicBlock *B, Value *V) {
        BBManage.add(B, V, FREED);
    }

    void FunctionInformation::addFreeValue(BasicBlock *B, Value *V, Type *memTy, Type * stTy, long num) {
        BBManage.add(B, V, memTy, stTy, num, FREED);
    }

    void FunctionInformation::addAllocValue(BasicBlock *B, Value *V) {
        BBManage.add(B, V, ALLOCATED);
    }

    void FunctionInformation::addAllocValue(BasicBlock *B, Value *V, Type *memTy, Type * stTy, long num) {
        BBManage.add(B, V, memTy, stTy, num, ALLOCATED);
    }

    bool FunctionInformation::isUnanalyzed(){
        return getStat() == UNANALYZED ? true : false;
    }

    bool FunctionInformation::isInProgress(){
        return getStat() == IN_PROGRESS ? true : false;
    }

    bool FunctionInformation::isAnalyzed(){
        return getStat() == ANALYZED ? true : false;
    }

    void FunctionInformation::setAnalyzed(){
        setStat(ANALYZED);
    }

    void FunctionInformation::setInProgress(){
        setStat(IN_PROGRESS);
    }

    void FunctionInformation::BBCollectInfo(BasicBlock& B, bool isEntryPoint){
        BBManage.CollectInInfo(&B, isEntryPoint);
    }

    void FunctionInformation::addFreedStruct(Type *T, Value *V){
        freedStruct.push_back(FreedStruct(T, V));
    }

    vector<BasicBlock *> FunctionInformation::getEndPoint() const{
        return endPoint;
    }

    FreedStructList FunctionInformation::getFreedStruct() const{
        return freedStruct;
    }
    BasicBlockList FunctionInformation::getFreeList(BasicBlock *B) {
        return BBManage.getBasicBlockFreeList(B);
    }

    BasicBlockList FunctionInformation::getAllocList(BasicBlock *B) {
        return BBManage.getBasicBlockAllocList(B);
    }

    bool FunctionInformation::isArgValue(Value *v){
        return args.isInList(v);
    }
    void FunctionInformation::setArgFree(Value *V){
        args.setFreed(V);
    }

    void FunctionInformation::setArgAlloc(Value *V){
        args.setAllocated(V);
    }
    bool FunctionInformation::isArgFreed(int64_t num){
        args.isArgFreed(num);
    }
    bool FunctionInformation::isArgAllocated(int64_t num){
        args.isArgAllocated(num);
    }
}
