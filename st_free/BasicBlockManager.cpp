#include "include/BasicBlockManager.hpp"

namespace ST_free {

    BasicBlockWorkList::BasicBlockWorkList(){
        MarkedValues = BasicBlockList();
    }

    BasicBlockWorkList::BasicBlockWorkList(const BasicBlockList v){
        MarkedValues = BasicBlockList(v);
    }

    // void BasicBlockWorkList::add(Value *v, Type * ty, long mem){
    //     UniqueKey uk(v, ty, mem);
    //     MarkedValues.push_back(uk);
    // }
    void BasicBlockWorkList::add(const UniqueKey *UK) {
        MarkedValues.push_back(UK);
    }
    // bool BasicBlockWorkList::exists(Value * v, Type * ty, long mem){
    //     if(find(MarkedValues.begin(), MarkedValues.end(), UniqueKey(v, ty, mem)) != MarkedValues.end())
    //         return true;
    //     return false;
    // }

    bool BasicBlockWorkList::exists(const UniqueKey *UK) {
        if(find(MarkedValues.begin(), MarkedValues.end(), UK) != MarkedValues.end())
            return true;
        return false;
    }

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

    // void BasicBlockInformation::addFree(Value *v, Type * ty , long mem){
    //     freeList.add(v, ty, mem);
    // }

    void BasicBlockInformation::addFree(const UniqueKey *UK){
        freeList.add(UK);
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

    // void BasicBlockInformation::addCorrectlyFreedValue(Value * V, Type * T, long mem){
    //     correctlyFreed.add(V, T, mem);
    // }

    void BasicBlockInformation::addCorrectlyFreedValue(const UniqueKey *UK){
        correctlyFreed.add(UK);
    }

    // bool BasicBlockInformation::CorrectlyFreedValueExists(Value * V, Type * T, long mem){
    //     return correctlyFreed.exists(V, T, mem);
    // }

    bool BasicBlockInformation::CorrectlyFreedValueExists(const UniqueKey *UK){
        return correctlyFreed.exists(UK);
    }

    BasicBlockWorkList BasicBlockInformation::getCorrectlyFreedValues() const{
        return correctlyFreed;
    }

    // bool BasicBlockInformation::FreeExists(Value *v, Type * ty, long mem){
    //     return freeList.exists(v, ty, mem);
    // }

    bool BasicBlockInformation::FreeExists(const UniqueKey *UK){
        return freeList.exists(UK);
    }

    void BasicBlockInformation::setFreeList(BasicBlockList v){
        freeList.setList(v);
    }

    // void BasicBlockInformation::addAlloc(Value *v, Type * ty, long mem){
    //     allocList.add(v, ty, mem);
    // }

    void BasicBlockInformation::addAlloc(const UniqueKey *UK) {
        allocList.add(UK);
    }

    bool BasicBlockInformation::LiveVariableExists(Value * v){
        if(find(liveVariables.begin(), liveVariables.end(), v) != liveVariables.end())
            return true;
        return false;
    }

    // bool BasicBlockInformation::AllocExists(Value *v, Type *ty, long mem){
    //     return allocList.exists(v, ty, mem);
    // }

    bool BasicBlockInformation::AllocExists(const UniqueKey *UK){
        return allocList.exists(UK);
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

    bool BasicBlockInformation::aliasExists(Value *vinfo){
        if(aliasMap.find(vinfo) != aliasMap.end())
            return true;
        return false;
    }

    Value* BasicBlockInformation::getAlias(Value *vinfo){
        if(this->aliasExists(vinfo)){
            return aliasMap[vinfo];
        }
        return NULL;
    }

    void BasicBlockInformation::setAlias(Value *src, Value *tgt){
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

        if(isEntryPoint)
            this->set(B);

        for (BasicBlock* PredBB: predecessors(B)) {
            if(isFirst) {
                this->copy(PredBB, B);
                isFirst = false;
            } else {
                this->intersect(PredBB, B);
                this->unite(PredBB, B);
                this->copyCorrectlyFreed(PredBB, B);
            }
        }
        return;
    }

    void BasicBlockManager::copy(BasicBlock *src, BasicBlock *tgt){
        BBMap[tgt] = BasicBlockInformation(BBMap[src]);
        return;
    }

    void BasicBlockManager::intersect(BasicBlock *src, BasicBlock *tgt){
        BBMap[tgt].setFreeList(intersectList(this->getBasicBlockFreeList(src), this->getBasicBlockFreeList(tgt)));
        BBMap[tgt].setLiveVariables(intersectLiveVariables(this->getLiveVariables(src), this->getLiveVariables(tgt)));
        return;
    }

    void BasicBlockManager::unite(BasicBlock *src, BasicBlock *tgt){
        BBMap[tgt].setAllocList(uniteList(this->getBasicBlockFreeList(src), this->getBasicBlockFreeList(tgt)));
        return;
    }

    void BasicBlockManager::copyCorrectlyFreed(BasicBlock *src, BasicBlock *tgt){
        for(const UniqueKey* uk : BBMap[src].getCorrectlyFreedValues().getList()){
            BBMap[tgt].addCorrectlyFreedValue(uk);
        }
    }

    // void BasicBlockManager::copyCorrectlyFreedToPrev(BasicBlock *src){
        // for (BasicBlock* PredBB: predecessors(src)) {
        //     BasicBlockInformation* BInfo = this->get(PredBB);
        //     if(BInfo && BInfo->isLoopBlock())
        //         this->copyCorrectlyFreed(src, PredBB);
        // }
    // }

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

    BasicBlockList BasicBlockManager::uniteList(BasicBlockList src, BasicBlockList tgt) {
        BasicBlockList tmp;
        llvm::sort(src.begin(), src.end());
        llvm::sort(tgt.begin(), tgt.end());

        set_union(
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

    LiveVariableList BasicBlockManager::getLiveVariables(BasicBlock *B){
        return BBMap[B].getLiveVariables();
    }

    bool BasicBlockManager::isPredBlockCorrectlyBranched(BasicBlock *B){
        if(pred_size(B) == 1){
            for (BasicBlock* PredBB: predecessors(B)) {
                if(BBMap[PredBB].isCorrectlyBranched())
                    return true;
            }
        }
        return false;
    }

    void BasicBlockManager::set(BasicBlock *B){
        if (!this->exists(B))
            BBMap[B] = BasicBlockInformation();
    }

    void BasicBlockManager::updateSuccessorBlock(BasicBlock *src){
        for(BasicBlock * SucBB : successors(src)){
            if(this->exists(SucBB)){
                // this->intersect(src, SucBB);
                this->copyCorrectlyFreed(src, SucBB);
            }
        }
    }
}
