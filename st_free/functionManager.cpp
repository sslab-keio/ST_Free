#include "functionManager.hpp"

namespace ST_free{
    bool FunctionManager::exists(Function *F) {
        if(func_map.find(F) != func_map.end())
            return true;
        return false;
    }

    FunctionInformation* FunctionManager::getElement(Function *F){
        if(!this->exists(F))
            func_map[F] = new FunctionInformation(F);
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
        ValueInformation * varinfo = this->getValueInfo(V, memTy);
        if(varinfo == NULL)
            varinfo = this->addVariable(V, memTy, stTy, num);
        else
            varinfo->addStructParams(stTy, num);
        // BBManage.add(B, V, memTy, stTy, num, FREED);
        BBManage.add(B, V, memTy, FREED);
    }

    void FunctionInformation::addAllocValue(BasicBlock *B, Value *V) {
        BBManage.add(B, V, ALLOCATED);
    }

    // void FunctionInformation::addAllocValue(BasicBlock *B, Value *V, Type *memTy, Type * stTy, long num) {
    //     BBManage.add(B, V, memTy, stTy, num, ALLOCATED);
    // }

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

    void FunctionInformation::addFreedStruct(Type *T, Value *V, Instruction *I){
        freedStruct.push_back(FreedStruct(T, V, I));
    }

    void FunctionInformation::addFreedStruct(BasicBlock *B, Type *T, Value *V, Instruction *I){
        freedStruct.push_back(FreedStruct(T, V, I, B, NULL));
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
    void FunctionInformation::setStructArgFree(Value *V, int64_t num){
        args.setFreedStructNumber(V, num);
    }
    void FunctionInformation::setStructArgAlloc(Value *V, int64_t num){
        args.setAllocatedStructNumber(V, num);
    }
    void FunctionInformation::setStructMemberArgFreed(Value *V, int64_t num){
        args.setStructMemberFree(V, num);
    }
    void FunctionInformation::setStructMemberArgAllocated(Value *V, int64_t num){
        args.setStructMemberAllocated(V, num);
    }
    bool FunctionInformation::isArgFreed(int64_t num){
        args.isArgFreed(num);
    }
    bool FunctionInformation::isArgAllocated(int64_t num){
        args.isArgAllocated(num);
    }
    ValueInformation * FunctionInformation::addVariable(Value * val){
        if(!VManage.exists(val))
            VManage.addValueInfo(val);
        return VManage.getValueInfo(val);
    }
    
    ValueInformation * FunctionInformation::addVariable(Value * val, Type * memType, Type *parType, long num){
        if(!VManage.exists(val, memType))
            VManage.addValueInfo(val, memType, parType, num);
        return VManage.getValueInfo(val, memType);
    }

    ValueInformation * FunctionInformation::getValueInfo(Value * val){
        return VManage.getValueInfo(val);
    }

    ValueInformation * FunctionInformation::getValueInfo(Value * val, Type * ty){
        return VManage.getValueInfo(val, ty);
    }

    void FunctionInformation::addLocalVar(BasicBlock *B, Type *T, Value * V, Instruction * I) {
        localVariables.push_back(FreedStruct(T, V, I));
    }

    void FunctionInformation::addLocalVar(BasicBlock *B, Type *T, Value * V, Instruction * I, ParentList P, ValueInformation *vinfo) {
        localVariables.push_back(FreedStruct(T, V, I, P, B, vinfo));
    }
    
    void FunctionInformation::incrementFreedRefCount(BasicBlock *B, Value *V, Value *ref){
        ValueInformation * vinfo = VManage.getValueInfo(V);
        if(vinfo != NULL)
            vinfo->incrementRefCount(ref);
    }

    void FunctionInformation::incrementAllocatedRefCount(BasicBlock *B, Value *V, Value *ref) {
        // BBManage.incrementRefCount(B, V, ref, ALLOCATED);
    }

    void FunctionInformation::decrementFreedRefCount(BasicBlock *B, Value *V, Value *ref){
        // BBManage.decrementRefCount(B, V, ref, FREED);
    }
    void FunctionInformation::decrementAllocatedRefCount(BasicBlock *B, Value *V, Value *ref){
        // BBManage.decrementRefCount(B, V, ref, ALLOCATED);
    }
    LocalVarList FunctionInformation::getLocalVar() const{
        return localVariables;
    }

    bool FunctionInformation::localVarExists(Type * T){
        if(find(localVariables.begin(), localVariables.end(), T) == localVariables.end())
            return false;
        return true;
    }
    void FunctionInformation::setStructMemberFreed(FreedStruct *fstruct, int64_t num){
        auto fs = find(freedStruct.begin(), freedStruct.end(), *fstruct);
        if(fs != freedStruct.end())
            fs->setFreedMember(num);
    }
    vector<bool> FunctionInformation::getStructMemberFreed(Type * T){
        auto fs = find(freedStruct.begin(), freedStruct.end(), T);
        if(fs != freedStruct.end())
            return fs->getFreedMember();
        return vector<bool>();
    }
    void FunctionInformation::copyStructMemberFreed(Type * T,vector<bool> members){
        auto fs = find(freedStruct.begin(), freedStruct.end(), T);
        if(fs != freedStruct.end())
            for(int ind = 0; ind != members.size(); ind++){
                if(members[ind])
                    fs->setFreedMember(ind);
            }
            //Copy vector member infos
        
    }
    void FunctionInformation::addBasicBlockLiveVariable(BasicBlock * B, Value *V){
        BBManage.addLiveVariable(B, V);
    }
}
