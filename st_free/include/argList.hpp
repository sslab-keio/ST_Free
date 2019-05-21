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
        public:
            ArgList(){
                arg_list = vector<Value *>();
            }
            explicit ArgList(uint64_t arg_num){
                argNum = arg_num;
                arg_list = vector<Value *>(arg_num, NULL);
                freeList = vector<bool>(arg_num, false);
                allocList = vector<bool>(arg_num, false);
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
    };
}
