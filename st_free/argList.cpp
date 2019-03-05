#include "ST_free.hpp"

namespace ST_free{
    class argList{
        private:
            vector<Value *> arg_list;
        public:
            explicit arg_list(uint64_t arg_num){
                arg_list = vector<Value *>(arg_num, NULL);
            }
            void setArg(uint64_t arg_no, Value * V);
            Value * getArg(uint64_t argno);
    };
}
