#include "include/functionManager.hpp"

namespace ST_free{
    bool FunctionManager::exists(Function *F) {
        if(func_map.find(F) != func_map.end())
            return true;
        return false;
    }

    void FreedStruct::print(){
        outs() << "\t[FreedMember]\n";
        for(int ind = 0; ind < FreedMembers.size(); ind++){
            outs() << "\t  [" << ind << "] ";
            if(FreedMembers[ind])
                outs() << "IsFreed\n";
            else
                outs() << "NotFreed\n";
        }
        return;
    }

    void FreedStruct::setStoredInLoop(int ind) {
        if(ind < storedInLoop.size())
            storedInLoop[ind] = true;
    }
    bool FreedStruct::isStoredInLoop(int ind) {
        if(ind < storedInLoop.size())
            return storedInLoop[ind];
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
        BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
        if(BInfo)
            BInfo->addFree(V, V->getType(), -1);
        // BBManage.add(B, V, FREED);
    }

    void FunctionInformation::addFreeValue(BasicBlock *B, Value *V, Type *memTy, Type * stTy, long num) {
        ValueInformation * varinfo = this->getValueInfo(V, memTy, num);
        BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);

        if(varinfo == NULL)
            varinfo = this->addVariable(V, memTy, stTy, num);
        else
            varinfo->addStructParams(stTy, num);
        
        varinfo->setFreed();

        if(BInfo){
            BInfo->addFree(V, memTy, num);
            if(BBManage.isPredBlockCorrectlyBranched(B) 
                    || BInfo->isLoopBlock()){
                // outs() << *varinfo->getValue() << " " << *varinfo->getMemberType() << "\n";
                this->addCorrectlyFreedValue(B, V, memTy, num);
            }
        }
    }

    void FunctionInformation::addAllocValue(BasicBlock *B, Value *V, Type * T, long mem) {
        BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
        if(BInfo)
            BInfo->addAlloc(V, T, mem);
        // BBManage.add(B, V, T, mem, ALLOCATED);
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

    void FunctionInformation::addFreedStruct(Type *T, Value *V, Instruction *I){
        freedStruct.push_back(new FreedStruct(T, V, I));
    }

    void FunctionInformation::addFreedStruct(BasicBlock *B, Type *T, Value *V, Instruction *I){
        FreedStruct * fst = new FreedStruct(T, V, I, B, NULL);
        if(!this->freedStructExists(fst)){
            freedStruct.push_back(fst);
        }
    }

    void FunctionInformation::addFreedStruct(BasicBlock *B, Type *T, Value *V, Instruction *I, StructType *parent){
        FreedStruct * fst = new FreedStruct(T, V, I, B, NULL);
        if(!this->freedStructExists(fst)){
            fst->addParent(parent);
            freedStruct.push_back(fst);
        }
    }

    bool FunctionInformation::freedStructExists(FreedStruct *fst){
        if(find(freedStruct.begin(), freedStruct.end(), fst) != freedStruct.end())
            return true;
        return false;
    }

    void FunctionInformation::addParentType(Type *T, Value *V, Instruction *I, StructType * parentTy){
        FreedStruct fst(T, V, I);
        auto fVal = find(freedStruct.begin(), freedStruct.end(), &fst);
        if(fVal != freedStruct.end() && parentTy != NULL)
            (*fVal)->addParent(parentTy);
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
        if(!VManage.exists(val, memType, num))
            VManage.addValueInfo(val, memType, parType, num);
        return VManage.getValueInfo(val, memType, num);
    }

    ValueInformation * FunctionInformation::getValueInfo(Value * val){
        return VManage.getValueInfo(val);
    }

    ValueInformation * FunctionInformation::getValueInfo(Value * val, Type * ty, long mem){
        return VManage.getValueInfo(val, ty, mem);
    }

    void FunctionInformation::addLocalVar(BasicBlock *B, Type *T, Value * V, Instruction * I) {
        localVariables.push_back(new FreedStruct(T, V, I));
    }

    void FunctionInformation::addLocalVar(BasicBlock *B, Type *T, Value * V, Instruction * I, ParentList P, ValueInformation *vinfo) {
        localVariables.push_back(new FreedStruct(T, V, I, P, B, vinfo));
    }

    void FunctionInformation::incrementRefCount(Value *V, Type *T, long mem, Value *ref){
        ValueInformation * vinfo = VManage.getValueInfo(V, T, mem);
        if(vinfo == NULL){
            VManage.addValueInfo(V, T, mem);
            vinfo = VManage.getValueInfo(V, T, mem);
        }
        vinfo->incrementRefCount(ref);
    }
    
    // void FunctionInformation::incrementFreedRefCount(BasicBlock *B, Value *V, Value *ref){
    //     ValueInformation * vinfo = VManage.getValueInfo(V);
    //     if(vinfo != NULL)
    //         vinfo->incrementRefCount(ref);
    // }

    LocalVarList FunctionInformation::getLocalVar() const{
        return localVariables;
    }

    bool FunctionInformation::localVarExists(Type * T){
        if(find_if(localVariables.begin(), localVariables.end(), [T](FreedStruct *fs) { return *fs == T; }) == localVariables.end())
            return false;
        return true;
    }

    void FunctionInformation::setStructMemberFreed(FreedStruct *fstruct, int64_t num){
        auto fs = find(freedStruct.begin(), freedStruct.end(), fstruct);
        if(fs != freedStruct.end()){
            (*fs)->setFreedMember(num);
        }
    }
    vector<bool> FunctionInformation::getStructMemberFreed(Type * T){
        auto fs = find_if(freedStruct.begin(), freedStruct.end(),  [T](FreedStruct *f) { return *f == T; });
        if(fs != freedStruct.end())
            return (*fs)->getFreedMember();
        return vector<bool>();
    }
    void FunctionInformation::copyStructMemberFreed(Type * T,vector<bool> members){
        auto fs = find_if(freedStruct.begin(), freedStruct.end(), [T](FreedStruct *f){return *f == T; });
        if(fs != freedStruct.end())
            for(int ind = 0; ind != members.size(); ind++){
                if(members[ind])
                    (*fs)->setFreedMember(ind);
            }
    }
    void FunctionInformation::addBasicBlockLiveVariable(BasicBlock * B, Value *V){
        BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
        if(BInfo)
            BInfo->addLiveVariable(V);
    }
    bool FunctionInformation::isFreedInBasicBlock(BasicBlock *B, Value *val, Type* ty, long mem){
        BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
        if(BInfo)
            return BInfo->FreeExists(val, ty, mem);
        return false;
    }
    bool FunctionInformation::isAllocatedInBasicBlock(BasicBlock *B, Value *val, Type* ty, long mem){
        BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
        if(BInfo)
            return BInfo->AllocExists(val, ty, mem);
        return false;
    }
    bool FunctionInformation::isLiveInBasicBlock(BasicBlock *B, Value *val){
        BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
        if(BInfo)
            return BInfo->LiveVariableExists(val);
    }
    void FunctionInformation::setCorrectlyBranched(BasicBlock *B){
        BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
        if(BInfo)
            BInfo->setCorrectlyBranched();
    }
    bool FunctionInformation::isCorrectlyBranched(BasicBlock *B){
        BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
        if(BInfo)
            return BInfo->isCorrectlyBranched();
        return false;
    }

    bool FunctionInformation::isPredBlockCorrectlyBranched(BasicBlock *B){
        return BBManage.isPredBlockCorrectlyBranched(B);
    }
    void FunctionInformation::addCorrectlyFreedValue(BasicBlock * B, Value * V,Type * T, long mem){
        BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
        if(BInfo)
            BInfo->addCorrectlyFreedValue(V, T, mem);
    }
    bool FunctionInformation::isCorrectlyBranchedFreeValue(BasicBlock *B, Value *V, Type *T, long mem){
        BasicBlockInformation *BInfo = this->getBasicBlockInformation(B);
        if(BInfo)
            return BInfo->CorrectlyFreedValueExists(V, T, mem);
        return false;
    }
    void FunctionInformation::setLoopBlock(BasicBlock &B){
        BasicBlockInformation *BInfo = this->getBasicBlockInformation(&B);
        if(LoopI->getLoopFor(&B)){
            if(BInfo)
                BInfo->setLoopBlock();
        }
    }
    bool FunctionInformation::isLoopBlock(BasicBlock &B){
        BasicBlockInformation *BInfo = this->getBasicBlockInformation(&B);
        if(BInfo)
            return BInfo->isLoopBlock();
        return false;
    }
    void FunctionInformation::setLoopInfo(LoopInfo *li){
        LoopI = li;
    }

    void FunctionInformation::setAliasInBasicBlock(BasicBlock *B, Value *srcinfo, Value *tgtinfo){
        BasicBlockInformation * BInfo = this->getBasicBlockInformation(B);
        if(BInfo)
            BInfo->setAlias(srcinfo, tgtinfo);
    }

    bool FunctionInformation::aliasExists(BasicBlock *B, Value *src){
        BasicBlockInformation * BInfo = this->getBasicBlockInformation(B);
        if(BInfo)
            return BInfo->aliasExists(src);
        return false;
    }

    Value * FunctionInformation::getAlias(BasicBlock *B, Value *src){
        BasicBlockInformation * BInfo = this->getBasicBlockInformation(B);
        if(BInfo)
            return BInfo->getAlias(src);
        return NULL;
    }

    BasicBlockInformation* FunctionInformation::getBasicBlockInformation(BasicBlock *B){
        return BBManage.get(B);
    }
}
