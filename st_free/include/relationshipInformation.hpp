#include "ST_free.hpp"

using namespace {
    using aliasList = Vector<Value *>;
    class relationshipInformation {
        private:
            map<Value *, aliasList> rmap;
        public:
            bool exists(Value *src);
            aliasList get(Value *src);
            void add(Value *src, Value *tgt);
            bool hasRelationship(Value *src, Value *tgt);
    };
}
