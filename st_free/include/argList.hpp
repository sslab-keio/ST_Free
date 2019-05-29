#pragma once
#include "ST_free.hpp"
#include "support_funcs.hpp"

namespace ST_free{
    class ArgList{
        private:
            uint64_t argNum;
            vector<Value *> arg_list;
            vector<bool> freeList;
            vector<bool> allocList;
            vector<vector<bool>> freedStructList;
            vector<vector<bool>> allocatedStructList;
        public:
            ArgList(){
                arg_list = vector<Value *>();
            }
            explicit ArgList(uint64_t arg_num){
                argNum = arg_num;
                arg_list = vector<Value *>(arg_num, NULL);
                freeList = vector<bool>(arg_num, false);
                allocList = vector<bool>(arg_num, false);
                freedStructList = vector<vector<bool>>(arg_num);
                allocatedStructList = vector<vector<bool>>(arg_num);
            }
            void setArg(uint64_t arg_no, Value * V);
            Value * getArg(uint64_t arg_no);
            void setArgs(Function *F);
            bool isInList(Value * V);
            int64_t getOperandNum(Value *V);
            void setFreed(Value *V);
            void setFreed(int64_t num);
            void setAllocated(Value *V);
            void setAllocated(int64_t num);
            bool isArgFreed(int64_t num);
            bool isArgAllocated(int64_t num);
            void setFreedStructNumber(int64_t arg, int64_t num);
            void setAllocatedStructNumber(int64_t arg, int64_t num);
            void setStructMemberFree(int64_t arg, int64_t num);
            void setStructMemberAllocated(int64_t arg, int64_t num);
            void setFreedStructNumber(Value * val, int64_t num);
            void setAllocatedStructNumber(Value * val, int64_t num);
            void setStructMemberFree(Value * val, int64_t num);
            void setStructMemberAllocated(Value * val, int64_t num);
    };
}
