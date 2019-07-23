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
            vector<bool> storedInLoop;
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
            bool operator ==(uniqueKey uk){
                return T == uk.getType() && V == uk.getValue();
            }
            bool operator !=(uniqueKey uk){
                return T != uk.getType() && V != uk.getValue();
            }

            Type * getType() const {return T;};
            Value * getValue() const {return V;};
            Instruction * getInst() const {return I;};
            void setFreedMember(int64_t num){FreedMembers[num] = true;};
            vector<bool> getFreedMember() const{return FreedMembers;};
            bool memberIsFreed(int ind) {return ind < FreedMembers.size() ? FreedMembers[ind]:false;};
            unsigned memberSize() {return FreedMembers.size();};
            BasicBlock * getFreedBlock() const{return freedBlock;};
            void addParent(StructType *st){ParentType.push_back(st);}
            Type *getTopParent(){return ParentType.size() >= 1 ? ParentType[0]:NULL;}
            void setStoredInLoop(int ind);
            bool isStoredInLoop(int ind);
            void print();
    };
    using FreedStructList = vector<FreedStruct *>;
    using LocalVarList = vector<FreedStruct *>;
    struct FunctionInformation {
        private:
            Function *F;
            int stat;
            ArgList args;
            vector<BasicBlock *> endPoint;
            LocalVarList localVariables;
            FreedStructList freedStruct;
            vector<uniqueKey> isCorrectlyBranchedFreeValues;
            BasicBlockManager BBManage;
            ValueManager VManage;
            LoopInfo * LoopI;
            map<Value *, vector<Function *>> funcPtr;
            int getStat();
            void setStat(int);
        public:
            /*** Costructor ***/
            FunctionInformation();
            FunctionInformation(Function *F);
            /*** Function ***/
            Function & getFunction();
            /*** EndPoints ***/
            void addEndPoint(BasicBlock *B);
            vector<BasicBlock *> getEndPoint() const;
            /*** FreeValue Related ***/
            void addFreeValue(BasicBlock *B, Value *V);
            void addFreeValue(BasicBlock *B, Value *V, Type *memTy, Type * stTy, long num);
            void incrementFreedRefCount(BasicBlock *B, Value *V, Value *refVal);
            void addFreedStruct(Type *T, Value *V, Instruction *I);
            void addFreedStruct(BasicBlock *B, Type *T, Value *V, Instruction *I);
            void addFreedStruct(BasicBlock *B, Type *T, Value *V, Instruction *I, StructType *parent);
            void addParentType(Type *T, Value *V,Instruction *I, StructType *parentTy);
            FreedStructList getFreedStruct() const;
            bool freedStructExists(FreedStruct *fst);
            /** AllocValue Related ***/
            void addAllocValue(BasicBlock *B, Value *V, Type *T, long mem);
            /*** Status Related ***/
            bool isUnanalyzed();
            bool isAnalyzed();
            bool isInProgress();
            void setAnalyzed();
            void setInProgress();
            /*** BasicBlock Related ***/
            BasicBlockInformation * getBasicBlockInformation(BasicBlock *B);
            void BBCollectInfo(BasicBlock& B, bool isEntryPoint);
            BasicBlockList getFreeList(BasicBlock *B);
            BasicBlockList getAllocList(BasicBlock *B);
            bool isFreedInBasicBlock(BasicBlock *B, Value * val, Type * ty, long mem);
            bool isAllocatedInBasicBlock(BasicBlock *B, Value * val, Type * ty, long mem);
            void addCorrectlyFreedValue(BasicBlock *, Value *, Type *, long mem);
            bool isCorrectlyBranchedFreeValue(BasicBlock *, Value *, Type *, long mem);
            void setCorrectlyBranched(BasicBlock *B);
            bool isCorrectlyBranched(BasicBlock *B);
            bool isPredBlockCorrectlyBranched(BasicBlock *B);
            void setAliasInBasicBlock(BasicBlock *B, Value *srcinfo, Value *tgtinfo);
            bool aliasExists(BasicBlock *B, Value * src);
            Value * getAlias(BasicBlock *B, Value *src);
            // void copyCorrectlyFreedValueInLoop(BasicBlock &B);
            void updateSuccessorBlock(BasicBlock &B);
            /*** Loop Related ***/
            void setLoopInfo(LoopInfo * li);
            void setLoopBlock(BasicBlock &B);
            bool isLoopBlock(BasicBlock &B);
            /*** Argument Values ***/
            bool isArgValue(Value *V);
            void setArgFree(Value *V);
            void setArgAlloc(Value *V);
            void setStructMemberFreed(FreedStruct * fstruct, int64_t num);
            vector<bool> getStructMemberFreed(Type * T);
            void copyStructMemberFreed(Type * T, vector<bool> members);
            void setStructArgFree(Value *V, int64_t num);
            void setStructArgAlloc(Value *V, int64_t num);
            void setStructMemberArgFreed(Value *V, int64_t num);
            void setStructMemberArgAllocated(Value *V, int64_t num);
            bool isArgFreed(int64_t num);
            bool isArgAllocated(int64_t num);
            /*** Individual Variable Informations ***/
            ValueInformation * addVariable(Value * val);
            ValueInformation * addVariable(Value * val, Type * memType, Type *parType, long num);
			ValueInformation * getValueInfo(Value * val);
			ValueInformation * getValueInfo(Value * val, Type * ty, long num);
            bool variableExists(Value *);
            void addLocalVar(BasicBlock *, Type *, Value *, Instruction *);
            void addLocalVar(BasicBlock *, Type *, Value *, Instruction *, ParentList P, ValueInformation *);
            LocalVarList getLocalVar() const;
            void addBasicBlockLiveVariable(BasicBlock *B, Value *);
            bool localVarExists(Type *);
            void incrementRefCount(Value *V, Type *T, long mem, Value *ref);
            bool isLiveInBasicBlock(BasicBlock *B, Value *val);
            /*** Debugging ***/
            void printVal(){VManage.print();}
            /*** Func Ptr related ***/
            void addFunctionPointerInfo(Value *val, Function *func);
            vector<Function *> getPointedFunctions(Value *val);
    };
    class FunctionManager {
        public:
            bool exists(Function *);
            FunctionInformation * getElement(Function *F);
        private:
            map<Function *, FunctionInformation *> func_map;
    };
}
