#pragma once
#include "ST_free.hpp"


namespace ST_free {
    using aliasList = vector<Value *>;
    struct AliasElement {
        StructType * stTy;
        int index;
        AliasElement(){
            stTy = NULL;
            index = -1;
        }
        AliasElement(StructType *st, int ind){
            stTy = st;
            index = ind;
        }
        bool operator<(const struct AliasElement &ae) const {
            if(this->stTy == ae.stTy)
                return this->index < ae.index;
            return this->stTy < ae.stTy;
        }
        bool operator==(const struct AliasElement &ae) const {
            return this->stTy == ae.stTy && this->index == ae.index;
        }
        void set(StructType * st, int ind){
            stTy = st;
            index = ind;
        }
    };
    class TypeRelationManager {
        public:
            bool exists(AliasElement src);
            void add(AliasElement src, AliasElement tgt);
            vector<AliasElement> getRelationshipList(AliasElement src);
            bool hadRelationship(AliasElement src, AliasElement tgt);
            ostream & operator<< (ostream &out);
            void print();
        private:
            map<AliasElement, vector<AliasElement>> typeMap;
    };
    class RelationshipInformation {
        public:
            bool exists(Value *src);
            aliasList get(Value *src);
            void add(Value *src, Value *tgt);
            bool hasRelationship(Value *src, Value *tgt);
        private:
            map<Value *, aliasList> rmap;
    };
}
