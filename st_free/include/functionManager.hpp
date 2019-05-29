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
        public:
            FreedStruct(){};
            FreedStruct(Type *Ty, Value *val, Instruction *Inst){
                T=Ty;
                V=val;
                I=Inst;
            };
            bool operator ==(Value * v){
                return V == v;
            }
            Type * getType() const {return T;};
            Value * getValue() const {return V;};
            Instruction * getInst() const {return I;};
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
            bool isArgFreed(int64_t num);
            bool isArgAllocated(int64_t num);
            void addLocalVar(Type *, Value *, Instruction *);
            LocalVarList getLocalVar() const;
    };

    class FunctionManager {
        private:
            map<Function *, FunctionInformation *> func_map;
        public:
            bool exists(Function *);
            FunctionInformation * getElement(Function *F);
    };
}
