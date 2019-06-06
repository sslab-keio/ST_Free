#pragma once
#include "ST_free.hpp"
#include "BBWorklist.hpp"
#include "argList.hpp"

namespace ST_free{
    using ParentList = vector<Type *>;
    struct FreedStruct {
        private:
            Type *T;
            ParentList ParentType;
            Value *V;
            Instruction *I;
            vector<bool> FreedMembers; 
            ValueInformation * valInfo;
            BasicBlock * freedBlock;
        public:
            FreedStruct(){};
            FreedStruct(Type *Ty, Value *val, Instruction *Inst){
                T=Ty;
                V=val;
                I=Inst;
                FreedMembers = vector<bool>(Ty->getStructNumElements(), false);
            };
            FreedStruct(Type *Ty, Value *val, Instruction *Inst, ParentList P){
                T=Ty;
                V=val;
                I=Inst;
                FreedMembers = vector<bool>(Ty->getStructNumElements(), false);
                ParentType = ParentList(P);
                valInfo = NULL;
            };
            FreedStruct(Type *Ty, Value *val, Instruction *Inst, BasicBlock * freedB, ValueInformation *vinfo){
                T=Ty;
                V=val;
                I=Inst;
                FreedMembers = vector<bool>(Ty->getStructNumElements(), false);
                valInfo = vinfo;
                freedBlock = freedB;
            };
            FreedStruct(Type *Ty, Value *val, Instruction *Inst, ParentList P, BasicBlock * freedB, ValueInformation *vinfo){
                T=Ty;
                V=val;
                I=Inst;
                FreedMembers = vector<bool>(Ty->getStructNumElements(), false);
                valInfo = vinfo;
                freedBlock = freedB;
            };
            bool operator ==(Value * v){
                return V == v;
            }
            bool operator ==(Type * t){
                return T == t;
            }
            bool operator ==(FreedStruct v){
                return V == v.getValue() && T == v.getType() && I == v.getInst();
            }
            bool operator !=(FreedStruct v){
                return V != v.getValue() && T != v.getType() && I != v.getInst();
            }
            bool operator ==(pair<Type *, Value *> t){
                return T == t.first && V == t.second;
            }
            bool operator !=(pair<Type *, Value *> t){
                return T != t.first && V != t.second;
            }
            Type * getType() const {return T;};
            Value * getValue() const {return V;};
            Instruction * getInst() const {return I;};
            void setFreedMember(int64_t num){FreedMembers[num] = true;};
            vector<bool> getFreedMember() const{return FreedMembers;};
            BasicBlock * getFreedBlock() const{return freedBlock;};
    };
    using FreedStructList = vector<FreedStruct>;
    using LocalVarList = vector<FreedStruct>;
    struct FunctionInformation {
        private:
            Function *F;
            int stat;
            ArgList args;
            vector<BasicBlock *> endPoint;
            LocalVarList localVariables;
            FreedStructList freedStruct;
            vector<pair<Value *, Type *>> isCorrectlyBranchedFreeValues;
            BasicBlockManager BBManage;
            ValueManager VManage;
            int getStat();
            void setStat(int);
        public:
            /*** Costructor ***/
            FunctionInformation();
            FunctionInformation(Function *F);
            /*** EndPoints ***/
            void addEndPoint(BasicBlock *B);
            vector<BasicBlock *> getEndPoint() const;
            /*** FreeValue Related ***/
            void addFreeValue(BasicBlock *B, Value *V);
            void addFreeValue(BasicBlock *B, Value *V, Type *memTy, Type * stTy, long num);
            void incrementFreedRefCount(BasicBlock *B, Value *V, Value *refVal);
            void decrementFreedRefCount(BasicBlock *B, Value *V, Value *refVal);
            void addFreedStruct(Type *T, Value *V, Instruction *I);
            void addFreedStruct(BasicBlock *B, Type *T, Value *V, Instruction *I);
            FreedStructList getFreedStruct() const;
            /** AllocValue Related ***/
            void addAllocValue(BasicBlock *B, Value *V);
            void addAllocValue(BasicBlock *B, Value *V, Type *memTy, Type * stTy, long num);
            void incrementAllocatedRefCount(BasicBlock *B, Value *V, Value *refVal);
            void decrementAllocatedRefCount(BasicBlock *B, Value *V, Value *refVal);
            /*** Status Related ***/
            bool isUnanalyzed();
            bool isAnalyzed();
            bool isInProgress();
            void setAnalyzed();
            void setInProgress();
            /*** Function/BasicBlock Related ***/
            Function & getFunction();
            void BBCollectInfo(BasicBlock& B, bool isEntryPoint);
            BasicBlockList getFreeList(BasicBlock *B);
            BasicBlockList getAllocList(BasicBlock *B);
            bool isFreedInBasicBlock(BasicBlock *B, Value * val, Type * ty);
            bool isAllocatedInBasicBlock(BasicBlock *B, Value * val, Type * ty);
            void addCorrectlyFreedValue(BasicBlock *, Value *,Type *);
            bool isCorrectlyBranchedFreeValue(BasicBlock *, Value *, Type *);
            /*** Argument Values ***/
            bool isArgValue(Value *V);
            void setArgFree(Value *V);
            void setArgAlloc(Value *V);
            void setStructMemberFreed(FreedStruct * fstruct, int64_t num);
            vector<bool> getStructMemberFreed(Type * T);
            void copyStructMemberFreed(Type * T, vector<bool> members);
            // void setStructMemberllocated(Type *T, Value *V, Instruction *I, int64_t num);
            void setStructArgFree(Value *V, int64_t num);
            void setStructArgAlloc(Value *V, int64_t num);
            void setStructMemberArgFreed(Value *V, int64_t num);
            void setStructMemberArgAllocated(Value *V, int64_t num);
            bool isArgFreed(int64_t num);
            bool isArgAllocated(int64_t num);
            ValueInformation * addVariable(Value * val);
            ValueInformation * addVariable(Value * val, Type * memType, Type *parType, long num);
			ValueInformation * getValueInfo(Value * val);
			ValueInformation * getValueInfo(Value * val, Type * ty);
            bool variableExists(Value *);
            void addLocalVar(BasicBlock *, Type *, Value *, Instruction *);
            void addLocalVar(BasicBlock *, Type *, Value *, Instruction *, ParentList P, ValueInformation *);
            LocalVarList getLocalVar() const;
            void addBasicBlockLiveVariable(BasicBlock *B, Value *);
            bool localVarExists(Type *);
            void incrementRefCount(Value *V, Type *T, Value *ref);
            bool isLiveInBasicBlock(BasicBlock *B, Value *val);
            void setCorrectlyBranched(BasicBlock *B);
            bool isCorrectlyBranched(BasicBlock *B);
            bool isPredBlockCorrectlyBranched(BasicBlock *B);
    };

    class FunctionManager {
        private:
            map<Function *, FunctionInformation *> func_map;
        public:
            bool exists(Function *);
            FunctionInformation * getElement(Function *F);
    };
}
