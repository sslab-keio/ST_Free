#pragma once
#include "ST_free.hpp"
#include "ValueInformation.hpp"

namespace ST_free {
    struct FreedStruct {
        public:
            FreedStruct(){};
            FreedStruct(Type *Ty, Value *val, Instruction *Inst){
                T=Ty;
                V=val;
                I=Inst;
                FreedMembers = vector<bool>(Ty->getStructNumElements(), false);
                storedInLoop = vector<bool>(Ty->getStructNumElements(), false);
            };
            FreedStruct(Type *Ty, Value *val, Instruction *Inst, ParentList P){
                T=Ty;
                V=val;
                I=Inst;
                FreedMembers = vector<bool>(Ty->getStructNumElements(), false);
                storedInLoop = vector<bool>(Ty->getStructNumElements(), false);
                ParentType = ParentList(P);
                valInfo = NULL;
            };
            FreedStruct(Type *Ty, Value *val, Instruction *Inst, BasicBlock * freedB, ValueInformation *vinfo){
                T=Ty;
                V=val;
                I=Inst;
                FreedMembers = vector<bool>(Ty->getStructNumElements(), false);
                storedInLoop = vector<bool>(Ty->getStructNumElements(), false);
                valInfo = vinfo;
                freedBlock = freedB;
            };
            FreedStruct(Type *Ty, Value *val, Instruction *Inst, ParentList P, BasicBlock * freedB, ValueInformation *vinfo){
                T=Ty;
                V=val;
                I=Inst;
                FreedMembers = vector<bool>(Ty->getStructNumElements(), false);
                storedInLoop = vector<bool>(Ty->getStructNumElements(), false);
                valInfo = vinfo;
                freedBlock = freedB;
            };
            bool operator ==(Value * v){
                return V == v;
            }
            bool operator ==(Type * t){
                return this->T == t;
            }
            bool operator ==(Type t){
                return this->T == &t;
            }
            bool operator ==(FreedStruct *v){
                return V == v->getValue() && T == v->getType() && I == v->getInst();
            }
            bool operator !=(FreedStruct *v){
                return V != v->getValue() && T != v->getType() && I != v->getInst();
            }
            bool operator ==(FreedStruct v){
                return V == v.getValue() && T == v.getType() && I == v.getInst();
            }
            bool operator !=(FreedStruct v){
                return V != v.getValue() && T != v.getType() && I != v.getInst();
            }
            bool operator ==(UniqueKey uk){
                return T == uk.getType() && V == uk.getValue();
            }
            bool operator !=(UniqueKey uk){
                return T != uk.getType() && V != uk.getValue();
            }
            bool memberIsFreed(int ind) {return ind < FreedMembers.size() ? FreedMembers[ind]:false;}
            unsigned memberSize() {return FreedMembers.size();}
            void setStoredInLoop(int ind);
            bool isStoredInLoop(int ind);
            void print();
            /*** getter/setter ***/
            Type * getType() const {return T;}
            Value * getValue() const {return V;}
            Instruction * getInst() const {return I;}
            ValueInformation *getValueInformation() {return valInfo;}
            void setFreedMember(int64_t num){FreedMembers[num] = true;};
            vector<bool> getFreedMember() const{return FreedMembers;};
            BasicBlock * getFreedBlock() const{return freedBlock;};
            void addParent(StructType *st, int ind){ParentType.push_back(pair<Type*, int>(st, ind));}
            // Type *getTopParent(){return this->getValueInformation()->getTopParent();}
            Type *getTopParent(){return ParentType.empty() ? NULL:ParentType[0].first;}
            ParentList getParentTypes() {return this->getValueInformation()->getParents();}
        private:
            Type *T;
            ParentList ParentType;
            Value *V;
            Instruction *I;
            vector<bool> storedInLoop;
            vector<bool> FreedMembers;
            ValueInformation * valInfo;
            BasicBlock * freedBlock;
    };
}
