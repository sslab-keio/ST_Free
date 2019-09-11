#include "ArgList.hpp"

namespace ST_free{
    bool ArgStatus::isMemberFreed(vector<int> indexes) {
        if (indexes.size() > 0) {
            int topIndex = indexes[0];
            indexes.erase(indexes.begin());

            if (topIndex < structStatus.size())
                return structStatus[topIndex].isMemberFreed(indexes);
            else
                return false;
        }
        return this->isFreed();
    }

    void ArgStatus::setMemberFreed(vector<int> indexes) {
        if (indexes.size() > 0) {
            int topIndex = indexes[0];
            indexes.erase(indexes.begin());

            if (topIndex >= this->maxSize()) 
                return;

            if (topIndex < structStatus.size())
                this->extendStatusSize(topIndex - structStatus.size());

            return structStatus[topIndex].setMemberFreed(indexes);
        }
        this->setFreed();
        return;
    }

    int ArgStatus::maxSize() {
        if (this->isStruct())
            return this->getType()->getStructNumElements();
        return 0;
    }

    void ArgStatus::extendStatusSize(int extSize) {
        for(int i = 0; i < extSize && this->size() < this->maxSize(); i++) {
            structStatus.push_back(ArgStatus(T->getStructElementType(this->size())));
        }
    }

    void ArgList::setArg(uint64_t arg_no, Value *V){
        if (arg_no < arg_list.size()) {
             arg_list[arg_no] = V;
        }
        return;
    }

    Value * ArgList::getArg(uint64_t arg_no){
        if(arg_no < arg_list.size()){
            return arg_list[arg_no];
        }
        return NULL;
    }

    void ArgList::setArgs(Function *F){
        int i = 0;
        for(auto args = F->arg_begin(); args != F->arg_end(); args++, i++){
            Value * v = getArgAlloca(args);
            if (v != NULL) {
                this->setArg(i, v);
            }
        }
        return;
    }

    bool ArgList::isInList(Value * V){
        if(find(arg_list.begin(), arg_list.end(), V) != arg_list.end()) {
            return true;
        }
        return false;
    }

    int64_t ArgList::getOperandNum(Value *V){
        auto ele = find(arg_list.begin(), arg_list.end(), V);
        if (ele != arg_list.end()) {
            return distance(arg_list.begin(), ele);
        }
        return -1;
    }

    void ArgList::setFreed(Value *V){
        this->setFreed(V, vector<int>());
        return;
    }

    void ArgList::setFreed(Value *V, vector<int> indexes) {
        int64_t ind = getOperandNum(V);
        if(ind >= 0 && ind < argNum)
            stats[ind].setMemberFreed(indexes);
        return;
    }

    bool ArgList::isFreed(Value *V, vector<int> indexes) {
        int64_t ind = getOperandNum(V);
        if(ind >= 0 && ind < argNum)
            return stats[ind].isMemberFreed(indexes);
        return false;
    }

//     void ArgList::setAllocated(Value *V){
//         int64_t ind = getOperandNum(V);
//         if(ind >= 0)
//             this->setAllocated(ind);
//         return;
//     }

//     void ArgList::setAllocated(int64_t num){
//         if(num < argNum)
//             allocList[num] = true;
//         return;
//     }

//     bool ArgList::isArgFreed(int64_t num){
//         if(num < argNum)
//             return freeList[num];
//         return false;
//     }

//     bool ArgList::isArgAllocated(int64_t num){
//         if(num < argNum)
//             return allocList[num];
//         return false;
//     }

//     void ArgList::setFreedStructNumber(int64_t arg, int64_t num){
//         freedStructList[arg] = vector<bool>(num, false);
//     }

//     void ArgList::setAllocatedStructNumber(int64_t arg, int64_t num){
//         allocatedStructList[arg] = vector<bool>(num, false);
//     }

//     void ArgList::setStructMemberFree(int64_t arg, int64_t num){
//         freedStructList[arg][num] = true;
//     }
//     void ArgList::setStructMemberAllocated(int64_t arg, int64_t num){
//         allocatedStructList[arg][num] = true;
//     }
//     void ArgList::setFreedStructNumber(Value * val, int64_t num){
//         int64_t arg = getOperandNum(val);
//         if(arg >= 0 && arg < argNum)
//             freedStructList[arg] = vector<bool>(num, false);
//     }

//     void ArgList::setAllocatedStructNumber(Value * val, int64_t num){
//         int64_t arg = getOperandNum(val);
//         if(arg >= 0)
//             allocatedStructList[arg] = vector<bool>(num, false);
//     }

//     void ArgList::setStructMemberFree(Value * val, int64_t num){
//         int64_t arg = getOperandNum(val);
//         if(arg >= 0 && arg < argNum)
//             if(num >= 0 && num < freedStructList[arg].size())
//                 freedStructList[arg][num] = true;
//     }

//     void ArgList::setStructMemberAllocated(Value * val, int64_t num){
//         int64_t arg = getOperandNum(val);
//         if(arg >= 0)
//             allocatedStructList[arg][num] = true;
//     }
}
