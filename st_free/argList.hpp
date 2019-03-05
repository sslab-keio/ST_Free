#include "ST_free.hpp"
#include "support_funcs.hpp"

namespace ST_free{
    class ArgList{
        private:
            vector<Value *> arg_list;
        public:
            ArgList(){
                arg_list = vector<Value *>();
            }
            explicit ArgList(uint64_t arg_num){
                arg_list = vector<Value *>(arg_num, NULL);
            }
            void setArg(uint64_t arg_no, Value * V);
            Value * getArg(uint64_t arg_no);
            void setArgs(Function &F);
    };
}
