#include "ST_free.hpp"
#pragma once
namespace ST_free {
    class StatusList{
        private:
            map<Type *, map<Value *, vector<int>>> st_tab;
        public:
            int getStat(Type *, Value *, uint64_t);
            void setStat(Type *, Value *, uint64_t, int);
            bool exists(Type *, Value *, uint64_t);
            bool exists(Type *, Value *);
            void setList(Type *, Value *);
            vector<int> *  getList(Type *, Value *);
            size_t getSize(Type *, Value *);
    };
}

