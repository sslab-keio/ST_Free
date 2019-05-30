#pragma once
#include "ST_free.hpp"
#include "BBWorklist.hpp"
#include "argList.hpp"

namespace ST_free{
    struct FreedStruct {
        private:
            Type *T;
            Value *V;
            Instruction *I;
            vector<bool> FreedMembers; 
        public:
            FreedStruct(){};
            FreedStruct(Type *Ty, Value *val, Instruction *Inst){
                T=Ty;
                V=val;
                I=Inst;
                FreedMembers = vector<bool>(Ty->getStructNumElements(), false);
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
            Type * getType() const {return T;};
            Value * getValue() const {return V;};
            Instruction * getInst() const {return I;};
            void setFreedMember(int64_t num){FreedMembers[num] = true;}
            vector<bool> getFreedMember() const{return FreedMembers;}
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
            BasicBlockManager BBManage;
            int getStat();
            void setStat(int);
        public:
            FunctionInformation();
            FunctionInformation(Function *F);
            void addEndPoint(BasicBlock *B);
            vector<BasicBlock *> getEndPoint() const;
            void addFreeValue(BasicBlock *B, Value *V);
            void addFreeValue(BasicBlock *B, Value *V, Type *memTy, Type * stTy, long num);
            void addAllocValue(BasicBlock *B, Value *V);
            void addAllocValue(BasicBlock *B, Value *V, Type *memTy, Type * stTy, long num);
            void addFreedStruct(Type *T, Value *V, Instruction *I);
            FreedStructList getFreedStruct() const;
            bool isUnanalyzed();
            bool isAnalyzed();
            bool isInProgress();
            void setAnalyzed();
            void setInProgress();
            Function & getFunction();
            void BBCollectInfo(BasicBlock& B, bool isEntryPoint);
            BasicBlockList getFreeList(BasicBlock *B);
            BasicBlockList getAllocList(BasicBlock *B);
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
            void addLocalVar(Type *, Value *, Instruction *);
            LocalVarList getLocalVar() const;
            bool localVarExists(Type *);
    };

    class FunctionManager {
        private:
            map<Function *, FunctionInformation *> func_map;
        public:
            bool exists(Function *);
            FunctionInformation * getElement(Function *F);
    };
}
