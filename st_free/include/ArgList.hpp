#pragma once
#include "ST_free.hpp"
#include "support_funcs.hpp"

namespace ST_free {
    /**
     * [Class] ArgStatus
     * Keeps Track of each function arguments, and its stats.
     * If the Argument is StructType, and modifies the member of that argument,
     * it add a new ArgStatus Instance to structStatus vector.
     */
    class ArgStatus {
        private:
            bool freed;
            vector<ArgStatus> structStatus;
            Type *T;
        public:
            ArgStatus(uint64_t structSize){
                freed = false;
                structStatus = vector<ArgStatus>(structSize);
                T = NULL;
            }
            ArgStatus(Type *Ty) {
                freed = false;
                T = Ty;
            }
            ArgStatus(){
                freed = false;
                T = NULL;
            }
            bool isFreed(){return freed;}
            void setFreed(){freed = true;}
            void setType(Type *Ty){T = Ty;}
            Type* getType(){return T;}
            bool isStruct(){return T && T->isStructTy();}
            int size(){return structStatus.size();}
            int maxSize();
            bool isMemberFreed(vector<int> indexes);
            void setMemberFreed(vector<int> indexes);
            void extendStatusSize(int extSize);
    };
    class ArgList {
        private:
            uint64_t argNum;
            vector<Value *> arg_list;
            vector<ArgStatus> stats;
        public:
            ArgList(){
                arg_list = vector<Value *>();
            }
            explicit ArgList(uint64_t arg_num){
                argNum = arg_num;
                arg_list = vector<Value *>(arg_num, NULL);
                stats = vector<ArgStatus>(arg_num);
            }
            void setArg(uint64_t arg_no, Value * V);
            Value * getArg(uint64_t arg_no);
            ArgStatus* getArgStatus(int arg){return (arg >= 0 && arg < stats.size())  ? &stats[arg]:NULL;};
            void setArgs(Function *F);
            bool isInList(Value * V);
            int64_t getOperandNum(Value *V);
            void setFreed(Value *V);
            void setFreed(Value *V, vector<int> indexes);
            bool isFreed(Value *V, vector<int> indexes);
    };
    // class ArgList {
    //     private:
    //         uint64_t argNum;
    //         vector<Value *> arg_list;
    //         vector<bool> freeList;
    //         vector<bool> allocList;
    //         vector<vector<bool>> freedStructList;
    //         vector<vector<bool>> allocatedStructList;
    //     public:
    //         ArgList(){
    //             arg_list = vector<Value *>();
    //         }
    //         explicit ArgList(uint64_t arg_num){
    //             argNum = arg_num;
    //             arg_list = vector<Value *>(arg_num, NULL);
    //             freeList = vector<bool>(arg_num, false);
    //             allocList = vector<bool>(arg_num, false);
    //             freedStructList = vector<vector<bool>>(arg_num);
    //             allocatedStructList = vector<vector<bool>>(arg_num);
    //         }
    //         void setArg(uint64_t arg_no, Value * V);
    //         Value * getArg(uint64_t arg_no);
    //         void setArgs(Function *F);
    //         bool isInList(Value * V);
    //         int64_t getOperandNum(Value *V);
    //         void setFreed(Value *V);
    //         void setFreed(int64_t num);
    //         void setAllocated(Value *V);
    //         void setAllocated(int64_t num);
    //         bool isArgFreed(int64_t num);
    //         bool isArgAllocated(int64_t num);
    //         void setFreedStructNumber(int64_t arg, int64_t num);
    //         void setAllocatedStructNumber(int64_t arg, int64_t num);
    //         void setStructMemberFree(int64_t arg, int64_t num);
    //         void setStructMemberAllocated(int64_t arg, int64_t num);
    //         void setFreedStructNumber(Value * val, int64_t num);
    //         void setAllocatedStructNumber(Value * val, int64_t num);
    //         void setStructMemberFree(Value * val, int64_t num);
    //         void setStructMemberAllocated(Value * val, int64_t num);
    // };
}
