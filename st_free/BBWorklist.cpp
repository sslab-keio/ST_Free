#include "include/BBWorklist.hpp"

namespace ST_free {
    BasicBlockWorkList::BasicBlockWorkList(){
        MarkedValues = BasicBlockList();
    }

    BasicBlockWorkList::BasicBlockWorkList(const BasicBlockList v){
        MarkedValues = BasicBlockList(v);
    }

    void BasicBlockWorkList::add(Value *v){
        MarkedValues.push_back(v);
    }

    bool BasicBlockWorkList::exists(Value * v){
        if(find(MarkedValues.begin(), MarkedValues.end(), v) != MarkedValues.end())
            return true;
        return false;
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

    bool BasicBlockStat::FreeExists(Value *v){
        return freeList.exists(v);
    }

    void BasicBlockStat::setFreeList(BasicBlockList v){
        freeList.setList(v);
    }

    void BasicBlockStat::addAlloc(Value *v){
        allocList.add(v);
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

    void BasicBlockManager::copy(BasicBlock *src, BasicBlock *tgt){
        BBMap[tgt] = BasicBlockStat(BBMap[src]);
        return;
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
