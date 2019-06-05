#include "include/BBWorklist.hpp"

namespace ST_free {

    BasicBlockWorkList::BasicBlockWorkList(){
        MarkedValues = BasicBlockList();
    }

    BasicBlockWorkList::BasicBlockWorkList(const BasicBlockList v){
        MarkedValues = BasicBlockList(v);
    }

    void BasicBlockWorkList::add(Value *v, Type * ty){
        MarkedValues.push_back(hashKeys(v, ty));
    }

    // void BasicBlockWorkList::add(Value * v, Type * memType, Type * structType, long memberNum){
    //     MarkedValues.push_back(ValueInformation(v, memType, structType, memberNum));
    // }

    bool BasicBlockWorkList::exists(Value * v, Type * ty){
        if(find(MarkedValues.begin(), MarkedValues.end(), hashKeys(v, ty)) != MarkedValues.end())
            return true;
        return false;
    }

    // void BasicBlockWorkList::incrementRefCount(Value *v, Value * refVal){
    //     auto MVal = find(MarkedValues.begin(), MarkedValues.end(), v);
    //     if(MVal != MarkedValues.end())
    //         MVal->incrementRefCount(refVal);
    // }

    // void BasicBlockWorkList::decrementRefCount(Value *v, Value * refVal){
    //     auto MVal = find(MarkedValues.begin(), MarkedValues.end(), v);
    //     if(MVal != MarkedValues.end())
    //         MVal->decrementRefCount();
    // }

    void BasicBlockWorkList::setList(BasicBlockList v){
        MarkedValues = BasicBlockList(v);
    }

    BasicBlockStat::BasicBlockStat(){
    }

    BasicBlockStat::BasicBlockStat(const BasicBlockStat& BStat){
        freeList = BasicBlockWorkList(BStat.getWorkList(FREED).getList());
        allocList = BasicBlockWorkList(BStat.getWorkList(ALLOCATED).getList());
        liveVariables = LiveVariableList(BStat.getLiveVariables());
    }

    BasicBlockWorkList BasicBlockStat::getWorkList(int mode) const {
        if(mode == FREED)
            return freeList;
        return allocList;
    }

    BasicBlockList BasicBlockWorkList::getList() const {
        return MarkedValues;
    }

    void BasicBlockStat::addFree(Value *v, Type * ty){
        freeList.add(v, ty);
    }

    // void BasicBlockStat::addFree(Value * v, Type * memType, Type * structType, long memberNum){
    //     freeList.add(v, memType, structType, memberNum);
    // }

    bool BasicBlockStat::FreeExists(Value *v, Type * ty){
        return freeList.exists(v, ty);
    }

    void BasicBlockStat::setFreeList(BasicBlockList v){
        freeList.setList(v);
    }

    void BasicBlockStat::addAlloc(Value *v, Type * ty){
        allocList.add(v, ty);
    }
    bool BasicBlockStat::LiveVariableExists(Value * v){
        if(find(liveVariables.begin(), liveVariables.end(), v) != liveVariables.end())
            return true;
        return false;
    }

    // void BasicBlockStat::addAlloc(Value * v, Type * memType, Type * structType, long memberNum){
    //     allocList.add(v, memType, structType, memberNum);
    // }

    // void BasicBlockStat::incrementFreedRefCount(Value *v, Value * refVal){
    //     freeList.incrementRefCount(v, refVal);
    // }
    // void BasicBlockStat::decrementFreedRefCount(Value *v, Value * refVal){
    //     freeList.decrementRefCount(v, refVal);
    // }
    // void BasicBlockStat::incrementAllocatedRefCount(Value *v, Value * refVal){
    //     allocList.incrementRefCount(v, refVal);
    // }
    // void BasicBlockStat::decrementAllocatedRefCount(Value *v, Value * refVal){
        // allocList.decrementRefCount(v, refVal);
    // }
    bool BasicBlockStat::AllocExists(Value *v, Type *ty){
        return allocList.exists(v, ty);
    }

    void BasicBlockStat::setAllocList(BasicBlockList v){
        allocList.setList(v);
    }

    void BasicBlockStat::addLiveVariable(Value *v){
        liveVariables.push_back(v);
    }

    LiveVariableList BasicBlockStat::getLiveVariables() const {
        return liveVariables;
    }
    void BasicBlockStat::setLiveVariables(LiveVariableList lvl){
        liveVariables = LiveVariableList(lvl);
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
            BBMap[B].addFree(v, v->getType());
        else if (mode == ALLOCATED)
            BBMap[B].addAlloc(v, v->getType());
        return;
    }

    void BasicBlockManager::add(BasicBlock *B, Value *v, Type * memTy, int mode){
        if (mode == FREED)
            BBMap[B].addFree(v, memTy);
        else if (mode == ALLOCATED)
            BBMap[B].addAlloc(v, memTy);
        return;
    }

    void BasicBlockManager::copy(BasicBlock *src, BasicBlock *tgt){
        BBMap[tgt] = BasicBlockStat(BBMap[src]);
        return;
    }
    // void BasicBlockManager::incrementRefCount(BasicBlock *B, Value *v, Value * refVal, int mode){
    //     if(mode == FREED)
    //         BBMap[B].incrementFreedRefCount(v, refVal);
    //     if(mode == ALLOCATED)
    //         BBMap[B].incrementAllocatedRefCount(v, refVal);
    // }

    // void BasicBlockManager::decrementRefCount(BasicBlock *B, Value *v, Value * refVal, int mode){
    //     if(mode == FREED)
    //         BBMap[B].decrementFreedRefCount(v, refVal);
    //     if(mode == ALLOCATED)
    //         BBMap[B].decrementAllocatedRefCount(v, refVal);
    // }

    void BasicBlockManager::intersect(BasicBlock *src, BasicBlock *tgt){
        BBMap[src].setFreeList(intersectList(this->getBasicBlockFreeList(src), this->getBasicBlockFreeList(tgt)));
        BBMap[src].setAllocList(intersectList(this->getBasicBlockAllocList(src), this->getBasicBlockAllocList(tgt)));
        BBMap[src].setLiveVariables(intersectLiveVariables(this->getLiveVariables(src), this->getLiveVariables(tgt)));
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
    LiveVariableList BasicBlockManager::intersectLiveVariables(LiveVariableList src, LiveVariableList tgt){
        LiveVariableList tmp;
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
    void BasicBlockManager::addLiveVariable(BasicBlock *B,Value *val){
        BBMap[B].addLiveVariable(val);
    }
    LiveVariableList BasicBlockManager::getLiveVariables(BasicBlock *B){
        return BBMap[B].getLiveVariables();
    }
    void BasicBlockManager::existsInFreedList(BasicBlock *B, Value *val, Type *ty){
        BBMap[B].FreeExists(val, ty);
    }
    void BasicBlockManager::existsInAllocatedList(BasicBlock *B, Value *val, Type *ty){
        BBMap[B].AllocExists(val, ty);
    }
    bool BasicBlockManager::existsInLiveVariableList(BasicBlock * B, Value *val){
        return BBMap[B].LiveVariableExists(val);
    }
}
