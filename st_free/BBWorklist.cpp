#include "include/BBWorklist.hpp"

namespace ST_free {

    BasicBlockWorkList::BasicBlockWorkList(){
        MarkedValues = BasicBlockList();
    }

    BasicBlockWorkList::BasicBlockWorkList(const BasicBlockList v){
        MarkedValues = BasicBlockList(v);
    }

    void BasicBlockWorkList::add(Value *v, Type * ty, long mem){
        MarkedValues.push_back(uniqueKey(v, ty, mem));
    }

    // void BasicBlockWorkList::add(Value * v, Type * memType, Type * structType, long memberNum){
    //     MarkedValues.push_back(ValueInformation(v, memType, structType, memberNum));
    // }

    bool BasicBlockWorkList::exists(Value * v, Type * ty, long mem){
        if(find(MarkedValues.begin(), MarkedValues.end(), uniqueKey(v, ty, mem)) != MarkedValues.end())
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

    BasicBlockInformation::BasicBlockInformation(){
        correctlyBranched = false;
        loopBlock = false;
    }

    BasicBlockInformation::BasicBlockInformation(const BasicBlockInformation& BStat){
        freeList = BasicBlockWorkList(BStat.getWorkList(FREED).getList());
        allocList = BasicBlockWorkList(BStat.getWorkList(ALLOCATED).getList());
        liveVariables = LiveVariableList(BStat.getLiveVariables());
        correctlyFreed = BasicBlockWorkList(BStat.getCorrectlyFreedValues());
        correctlyBranched = false;
        loopBlock = false;
    }

    BasicBlockWorkList BasicBlockInformation::getWorkList(int mode) const {
        if(mode == FREED)
            return freeList;
        return allocList;
    }

    BasicBlockList BasicBlockWorkList::getList() const {
        return MarkedValues;
    }

    void BasicBlockInformation::addFree(Value *v, Type * ty , long mem){
        freeList.add(v, ty, mem);
    }

    void BasicBlockInformation::setCorrectlyBranched(){
        correctlyBranched = true;
    }

    void BasicBlockInformation::setLoopBlock(){
        loopBlock = true;
    }

    bool BasicBlockInformation::isLoopBlock(){
        return loopBlock;
    }

    bool BasicBlockInformation::isCorrectlyBranched(){
        return correctlyBranched;
    }

    void BasicBlockInformation::addCorrectlyFreedValue(Value * V, Type * T, long mem){
        correctlyFreed.add(V, T, mem);
    }

    bool BasicBlockInformation::CorrectlyFreedValueExists(Value * V, Type * T, long mem){
        return correctlyFreed.exists(V, T, mem);
    }

    BasicBlockWorkList BasicBlockInformation::getCorrectlyFreedValues() const{
        return correctlyFreed;
    }

    // void BasicBlockInformation::addFree(Value * v, Type * memType, Type * structType, long memberNum){
    //     freeList.add(v, memType, structType, memberNum);
    // }

    bool BasicBlockInformation::FreeExists(Value *v, Type * ty, long mem){
        return freeList.exists(v, ty, mem);
    }

    void BasicBlockInformation::setFreeList(BasicBlockList v){
        freeList.setList(v);
    }

    void BasicBlockInformation::addAlloc(Value *v, Type * ty, long mem){
        allocList.add(v, ty, mem);
    }

    bool BasicBlockInformation::LiveVariableExists(Value * v){
        if(find(liveVariables.begin(), liveVariables.end(), v) != liveVariables.end())
            return true;
        return false;
    }

    bool BasicBlockInformation::AllocExists(Value *v, Type *ty, long mem){
        return allocList.exists(v, ty, mem);
    }

    void BasicBlockInformation::setAllocList(BasicBlockList v){
        allocList.setList(v);
    }

    void BasicBlockInformation::addLiveVariable(Value *v){
        liveVariables.push_back(v);
    }

    LiveVariableList BasicBlockInformation::getLiveVariables() const {
        return liveVariables;
    }
    void BasicBlockInformation::setLiveVariables(LiveVariableList lvl){
        liveVariables = LiveVariableList(lvl);
    }

    bool BasicBlockInformation::aliasExists(uniqueKey *vinfo){
        if(aliasMap.find(vinfo) != aliasMap.end())
            return true;
        return false;
    }

    uniqueKey* BasicBlockInformation::getAlias(uniqueKey *vinfo){
        if(this->aliasExists(vinfo)){
            return aliasMap[vinfo];
        }
        return NULL;
    }

    void BasicBlockInformation::setAlias(uniqueKey *src, uniqueKey *tgt){
        if(!this->aliasExists(tgt))
            aliasMap[tgt] = src;
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

    // void BasicBlockManager::add(BasicBlock *B, Value *v, int mode){
    //     if (mode == FREED)
    //         BBMap[B].addFree(v, v->getType(), -1);
    //     else if (mode == ALLOCATED)
    //         BBMap[B].addAlloc(v, v->getType(), -1);
    //     return;
    // }

    // void BasicBlockManager::add(BasicBlock *B, Value *v, Type * memTy, long mem, int mode){
    //     if (mode == FREED)
    //         BBMap[B].addFree(v, memTy, mem);
    //     else if (mode == ALLOCATED)
    //         BBMap[B].addAlloc(v, memTy, mem);
    //     return;
    // }

    void BasicBlockManager::copy(BasicBlock *src, BasicBlock *tgt){
        BBMap[tgt] = BasicBlockInformation(BBMap[src]);
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
                tgt.begin(), tgt.end(),
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

    BasicBlockInformation * BasicBlockManager::get(BasicBlock *B){
        if (this->exists(B))
            return &BBMap[B];
        return NULL;
    }
    // void BasicBlockManager::addLiveVariable(BasicBlock *B,Value *val){
    //     BBMap[B].addLiveVariable(val);
    // }
    LiveVariableList BasicBlockManager::getLiveVariables(BasicBlock *B){
        return BBMap[B].getLiveVariables();
    }
    // bool BasicBlockManager::existsInFreedList(BasicBlock *B, Value *val, Type *ty, long mem){
    //     return BBMap[B].FreeExists(val, ty, mem);
    // }
    // bool BasicBlockManager::existsInAllocatedList(BasicBlock *B, Value *val, Type *ty, long mem){
    //     return BBMap[B].AllocExists(val, ty, mem);
    // }
    // bool BasicBlockManager::existsInLiveVariableList(BasicBlock * B, Value *val){
    //     return BBMap[B].LiveVariableExists(val);
    // }

    // void BasicBlockManager::setCorrectlyBranched(BasicBlock *B){
    //     BBMap[B].setCorrectlyBranched();
    // }
    // bool BasicBlockManager::isCorrectlyBranched(BasicBlock *B){
    //     return BBMap[B].isCorrectlyBranched();
    // }
    // void BasicBlockManager::setLoopBlock(BasicBlock *B){
    //     BBMap[B].setLoopBlock();
    // }
    // bool BasicBlockManager::isLoopBlock(BasicBlock *B){
    //     return BBMap[B].isLoopBlock();
    // }
    bool BasicBlockManager::isPredBlockCorrectlyBranched(BasicBlock *B){
        if(pred_size(B) == 1){
            for (BasicBlock* PredBB: predecessors(B)) {
                if(BBMap[PredBB].isCorrectlyBranched())
                    return true;
            }
        }
        return false;
    }
    // void BasicBlockManager::addCorrectlyFreedValue(BasicBlock *B, Value * V, Type * T, long mem){
    //     BBMap[B].addCorrectlyFreedValue(V, T, mem);
    // }
    // bool BasicBlockManager::correctlyFreedValueExists(BasicBlock *B, Value * V, Type * T, long mem){
    //     return BBMap[B].CorrectlyFreedValueExists(V, T, mem);
    // }
}
