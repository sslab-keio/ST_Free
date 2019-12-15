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

    bool BasicBlockWorkList::typeExists(Type *T) {
        auto foundVal = find_if(MarkedValues.begin(), MarkedValues.end(),
                [T](const UniqueKey *UK) {
                    return UK->getType() == T;
                });
        if(foundVal != MarkedValues.end())
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

    BasicBlockWorkList BasicBlockInformation::getDMZList() const {
        return dmzList;
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

    void BasicBlockInformation::setDMZList(BasicBlockList v){
        dmzList.setList(v);
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

    bool BasicBlockManager::exists(BasicBlock *B){
        if(BBMap.find(B) != BBMap.end())
            return true;
        return false;
    }

    void BasicBlockManager::CollectInInfo(BasicBlock *B, bool isEntryPoint) {
        bool isFirst = true;
        if(this->exists(B))
            return;

        if(isEntryPoint)
            this->set(B);

        this->addFreeInfoFromDMZToPreds(B);
        for (BasicBlock* PredBB: predecessors(B)) {
            if(isFirst) {
                this->copy(PredBB, B);
                isFirst = false;
            } else {
                this->unite(PredBB, B);
                this->intersect(PredBB, B);
                this->copyCorrectlyFreed(PredBB, B);
            }
            this->diff(PredBB, B);
        }
        return;
    }

    void BasicBlockManager::copy(BasicBlock *src, BasicBlock *tgt){
        BBMap[tgt] = BasicBlockInformation(BBMap[src]);
        BBMap[tgt].setDMZList(this->getBasicBlockRemoveAllocList(src, tgt));
        return;
    }

    void BasicBlockManager::intersect(BasicBlock *src, BasicBlock *tgt){
        BBMap[tgt].setFreeList(intersectList(this->getBasicBlockFreeList(src), this->getBasicBlockFreeList(tgt)));
        // BBMap[tgt].setLiveVariables(intersectLiveVariables(this->getLiveVariables(src), this->getLiveVariables(tgt)));
        return;
    }

    void BasicBlockManager::unite(BasicBlock *src, BasicBlock *tgt){
        BBMap[tgt].setAllocList(uniteList(this->getBasicBlockAllocList(src), this->getBasicBlockAllocList(tgt)));
        BBMap[tgt].setDMZList(uniteList(this->getBasicBlockDMZList(tgt), this->getBasicBlockRemoveAllocList(src, tgt)));
        return;
    }

    void BasicBlockManager::diff(BasicBlock *src, BasicBlock *tgt){
        BBMap[tgt].setAllocList(diffList(this->getBasicBlockAllocList(tgt), this->get(src)->getRemoveAllocs(tgt).getList()));
        return;
    }

    void BasicBlockManager::nullCheckedToFree(BasicBlock *src, BasicBlock *tgt){
        outs() << this->get(tgt)->getDMZList().getList().size() << "\n";
        for (auto ele : this->getBasicBlockFreeList(src)) {
            if (this->get(tgt)->getDMZList().typeExists(ele->getType())) {
                outs() << *ele->getType() << "!\n";
            }
            // auto freedVal = find(this->getBasicBlockDMZList(tgt).begin(),
            //         this->getBasicBlockDMZList(tgt).end(),
            //         ele);
            // if (freedVal != this->getBasicBlockAllocList(tgt).end()) {
            //     outs() << *ele->getType() << "!\n";
            // }
        }
        return;
    }

    void BasicBlockManager::addFreeInfoFromDMZToPreds(BasicBlock *src) {
        BasicBlockList freeUnite;
        for (BasicBlock* PredBB: predecessors(src)) {
            freeUnite = uniteList(freeUnite, this->getBasicBlockFreeList(PredBB));
        }

        for (BasicBlock* PredBB: predecessors(src)) {
            BasicBlockInformation *BBInfo = this->get(PredBB);
            if (BBInfo) {
                for (auto ele : freeUnite) {
                    if (BBInfo->getDMZList().typeExists(ele->getType())) {
                        BBInfo->addFree(ele);
                    }
                }
            }
        }
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

    BasicBlockList BasicBlockManager::intersectList(BasicBlockList src, BasicBlockList tgt) {
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

    BasicBlockList BasicBlockManager::diffList(BasicBlockList src, BasicBlockList tgt) {
        BasicBlockList tmp;
        llvm::sort(src.begin(), src.end());
        llvm::sort(tgt.begin(), tgt.end());

        set_difference(
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

    BasicBlockList BasicBlockManager::getBasicBlockDMZList(BasicBlock *src) {
        if(this->exists(src))
            return BBMap[src].getDMZList().getList();
        return BasicBlockList();
    }

    BasicBlockList BasicBlockManager::getBasicBlockRemoveAllocList(BasicBlock *src, BasicBlock *tgt) {
        if(this->exists(src))
            return BBMap[src].getRemoveAllocs(tgt).getList();
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
            if(this->exists(SucBB)) {
                // this->intersect(src, SucBB);
                this->copyCorrectlyFreed(src, SucBB);
            }
        }
    }

    void BasicBlockInformation::addStoredCallValues(Value *v, CallInst *CI)  {
        storedCallValues.push_back(pair<Value *, CallInst *>(v, CI));
    }

    vector<pair<Value *, CallInst *>> BasicBlockInformation::getStoredCallValues() {
        return storedCallValues;
    }

    bool BasicBlockInformation::isCallValues(Value *V) {
        auto fele = find_if(storedCallValues.begin(), storedCallValues.end(),
                [V](pair<Value*, CallInst*> &p) {
                    return p.first == V;
                });
        if (fele != storedCallValues.end())
            return true;
        return false;
    }

    CallInst *BasicBlockInformation::getCallInstForVal(Value *V) {
        auto fele = find_if(storedCallValues.begin(), storedCallValues.end(),
                [V](pair<Value*, CallInst*> &p) {
                    return p.first == V;
                });
        if (fele != storedCallValues.end())
            return (*fele).second;
        return NULL;
    }
    void BasicBlockInformation::addRemoveAlloc(BasicBlock *B, UniqueKey *UK) {
        if (!B || !UK)
            return;
        if (removeAllocs.find(B) == removeAllocs.end())
            removeAllocs[B] = BasicBlockWorkList();
        removeAllocs[B].add(UK);
        return;
    }

    BasicBlockWorkList BasicBlockInformation::getRemoveAllocs(BasicBlock *B){
        if (removeAllocs.find(B) != removeAllocs.end())
            return removeAllocs[B];
        return BasicBlockWorkList();
    }
}
