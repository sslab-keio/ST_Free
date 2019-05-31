#include "include/BBWorklist.hpp"

namespace ST_free {
    Value * ValueInformation::getValue() const{
        return V;
    }
    Type * ValueInformation::getStructType() const{
        return structType;
    }

    Type * ValueInformation::getMemberType() const{
        return memberType;
    }

    long ValueInformation::getMemberNum() const{
        return memberNum;
    }

    bool ValueInformation::isStructMember(){
        if (memberType == NULL && structType == NULL)
            return false;
        return true;
    }

    BasicBlockWorkList::BasicBlockWorkList(){
        MarkedValues = BasicBlockList();
    }

    BasicBlockWorkList::BasicBlockWorkList(const BasicBlockList v){
        MarkedValues = BasicBlockList(v);
    }

    void BasicBlockWorkList::add(Value *v){
        MarkedValues.push_back(v);
    }

    void BasicBlockWorkList::add(Value * v, Type * memType, Type * structType, long memberNum){
        MarkedValues.push_back(ValueInformation(v, memType, structType, memberNum));
    }

    bool BasicBlockWorkList::exists(Value * v){
        if(find(MarkedValues.begin(), MarkedValues.end(), v) != MarkedValues.end())
            return true;
        return false;
    }

    void BasicBlockWorkList::incrementRefCount(Value *v, Value * refVal){
        auto MVal = find(MarkedValues.begin(), MarkedValues.end(), v);
        if(MVal != MarkedValues.end())
            MVal->incrementRefCount(refVal);
    }

    void BasicBlockWorkList::decrementRefCount(Value *v, Value * refVal){
        auto MVal = find(MarkedValues.begin(), MarkedValues.end(), v);
        if(MVal != MarkedValues.end())
            MVal->decrementRefCount();
    }

    void BasicBlockWorkList::setList(BasicBlockList v){
        MarkedValues = BasicBlockList(v);
    }

    BasicBlockStat::BasicBlockStat(){
    }

    BasicBlockStat::BasicBlockStat(const BasicBlockStat& BStat){
        freeList = BasicBlockWorkList(BStat.getWorkList(FREED).getList());
        allocList = BasicBlockWorkList(BStat.getWorkList(ALLOCATED).getList());
    }

    BasicBlockWorkList BasicBlockStat::getWorkList(int mode) const {
        if(mode == FREED)
            return freeList;
        return allocList;
    }

    BasicBlockList BasicBlockWorkList::getList() const {
        return MarkedValues;
    }

    void BasicBlockStat::addFree(Value *v){
        freeList.add(v);
    }

    void BasicBlockStat::addFree(Value * v, Type * memType, Type * structType, long memberNum){
        freeList.add(v, memType, structType, memberNum);
    }

    bool BasicBlockStat::FreeExists(Value *v){
        return freeList.exists(v);
    }

    void BasicBlockStat::setFreeList(BasicBlockList v){
        freeList.setList(v);
    }

    void BasicBlockStat::addAlloc(Value *v){
        allocList.add(v);
    }

    void BasicBlockStat::addAlloc(Value * v, Type * memType, Type * structType, long memberNum){
        allocList.add(v, memType, structType, memberNum);
    }

    void BasicBlockStat::incrementFreedRefCount(Value *v, Value * refVal){
        freeList.incrementRefCount(v, refVal);
    }
    void BasicBlockStat::decrementFreedRefCount(Value *v, Value * refVal){
        freeList.decrementRefCount(v, refVal);
    }
    void BasicBlockStat::incrementAllocatedRefCount(Value *v, Value * refVal){
        allocList.incrementRefCount(v, refVal);
    }
    void BasicBlockStat::decrementAllocatedRefCount(Value *v, Value * refVal){
        allocList.decrementRefCount(v, refVal);
    }
    bool BasicBlockStat::AllocExists(Value *v){
        return allocList.exists(v);
    }

    void BasicBlockStat::setAllocList(BasicBlockList v){
        allocList.setList(v);
    }

    bool BasicBlockManager::exists(BasicBlock *B){
        if(BBMap.find(B) != BBMap.end())
            return true;
        return false;
    }

    void BasicBlockManager::CollectInInfo(BasicBlock *B, bool isEntryPoint){
        bool isFirst = true;
        if(this->exists(B))
            return;
        for (BasicBlock* PredBB: predecessors(B)) {
            if(isFirst) {
                this->copy(PredBB, B);
                isFirst = false;
            } else {
                this->intersect(PredBB, B);
            }
        }
        return;
    }

    void BasicBlockManager::add(BasicBlock *B, Value *v, int mode){
        if (mode == FREED)
            BBMap[B].addFree(v);
        else if (mode == ALLOCATED)
            BBMap[B].addAlloc(v);
        return;
    }

    void BasicBlockManager::add(BasicBlock *B, Value *v, Type * memTy, Type* stTy, long num, int mode){
        if (mode == FREED)
            BBMap[B].addFree(v, memTy, stTy, num);
        else if (mode == ALLOCATED)
            BBMap[B].addAlloc(v, memTy, stTy, num);
        return;
    }

    void BasicBlockManager::copy(BasicBlock *src, BasicBlock *tgt){
        BBMap[tgt] = BasicBlockStat(BBMap[src]);
        return;
    }
    void BasicBlockManager::incrementRefCount(BasicBlock *B, Value *v, Value * refVal, int mode){
        if(mode == FREED)
            BBMap[B].incrementFreedRefCount(v, refVal);
        if(mode == ALLOCATED)
            BBMap[B].incrementAllocatedRefCount(v, refVal);
    }

    void BasicBlockManager::decrementRefCount(BasicBlock *B, Value *v, Value * refVal, int mode){
        if(mode == FREED)
            BBMap[B].decrementFreedRefCount(v, refVal);
        if(mode == ALLOCATED)
            BBMap[B].decrementAllocatedRefCount(v, refVal);
    }

    void BasicBlockManager::intersect(BasicBlock *src, BasicBlock *tgt){
        BBMap[src].setFreeList(intersectList(this->getBasicBlockFreeList(src), this->getBasicBlockFreeList(tgt)));
        BBMap[src].setAllocList(intersectList(this->getBasicBlockAllocList(src), this->getBasicBlockAllocList(tgt)));

        return;
    }

    BasicBlockList BasicBlockManager::intersectList(BasicBlockList src, BasicBlockList tgt){
        BasicBlockList tmp;
        llvm::sort(src.begin(), src.end());
        llvm::sort(tgt.begin(), tgt.end());

        set_intersection(
                src.begin(), src.end(),
                tgt.begin(),tgt.end(),
                back_inserter(tmp)
            );

        return tmp;
    }

    BasicBlockList BasicBlockManager::getBasicBlockFreeList(BasicBlock *src) {
        if(this->exists(src)){
            return BBMap[src].getWorkList(FREED).getList();
        }
        return BasicBlockList();
    }

    BasicBlockList BasicBlockManager::getBasicBlockAllocList(BasicBlock *src) {
        if(this->exists(src)){
            return BBMap[src].getWorkList(ALLOCATED).getList();
        }
        return BasicBlockList();
    }

    BasicBlockStat * BasicBlockManager::get(BasicBlock *B){
        if (this->exists(B))
            return &BBMap[B];
        return NULL;
    }
}
