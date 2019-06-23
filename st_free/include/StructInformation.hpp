#pragma once
#include "ST_free.hpp"
#include "determinator.hpp"
#include "support_funcs.hpp"
#include "functionManager.hpp"

#define ISRESPONSIBLE 1
#define ISNOTRESPONSIBLE 2
#define ISUNKNOWN 3
#define NOTPOINTERTY 4
#define UNALLOCATED 5

namespace ST_free{
    class CandidateValue{
        private:
            Function *F;
            FreedStruct *fst;
        public:
            CandidateValue(Function *func, FreedStruct *fs){F = func; fst = fs;}
            FreedStruct * getFreedStruct(){return fst;};
            Function * getFunction(){return F;};
            unsigned getMemberSize(){return fst->memberSize();};
            bool memberIsFreed(unsigned ind){return fst->memberIsFreed(ind);};
            Instruction * getInstruction(){return fst->getInst();}
            void print(){fst->print();};
    };
    /* Class [StructInformation]
     * keeps track of each structure information,
     * including referees, candidates, each memberstats,
     * and allocated times
     * */
    class StructInformation {
        private:
            struct storeCount {
                int total;
                int globalVar;
                storeCount(){
                    total = 0;
                    globalVar = 0;
                }
            };
            StructType * strTy;
            vector<StructType *> referees;
            vector<int> memberStats;
            vector<int> freedCounts;
            vector<storeCount> stc;
            int candidateNum;
            unsigned int allocNum;
            vector<CandidateValue *> candidates;
            bool isResponsible(int ind){return memberStats[ind] == ISRESPONSIBLE;};
            bool isUnknown(int ind){return memberStats[ind] == ISUNKNOWN;};
            bool judgeResponsibility(int ind);
            unsigned int getAllocNum(){return allocNum;};
            bool isAllStoreGlobalVar(int ind){return ((stc[ind].total > 0) && stc[ind].total - stc[ind].globalVar) == 0;}
        public:
            StructInformation(){};
            StructInformation(StructType * st);
            void addReferee(StructType * st);
            bool hasSingleReferee();
            void setMemberStatResponsible(int num);
            void setMemberStatNotResponsible(int num);
            void setMemberStatNotAllocated(int num);
            void addCandidateValue(Function *F, FreedStruct *fs);
            void print();
            void BuildCandidateCount();
            void checkCorrectness();
            vector<CandidateValue *> getCandidateValue() {return candidates;};
            void incrementFreedCount(int ind);
            void incrementAllocNum(){allocNum++;}
            bool hasNoAlloc(){return allocNum == 0;}
            void incrementStoreTotal(int ind);
            void incrementStoreGlobalVar(int ind);
    };
    /* Class
     * [Struct Manager]
     * Manages StructInformation in a map, and controls them
     * This manager should be generated per module.
     * The constructor generates the map of the struct, and also
     * stores the referees of each StructType. 
     * */
    class StructManager{
        private:
            map<StructType *, StructInformation *> StructInfo;
            void addReferee(StructType *referee, StructType *tgt);
            void createDependencies();
            void changeStats();
            bool exists(StructType *);
            void checkNonAllocs();
        public:
            StructManager(){};
            StructManager(vector<StructType *> st);
            StructInformation * get(StructType * strTy){return StructInfo[strTy];}
            void addCandidateValue(Function *F, StructType *strTy, FreedStruct * fs);
            void addAlloc(StructType *strTy);
            void addStore(StructType *strTy, int ind);
            void addGlobalVarStore(StructType *strTy, int ind);
            void print();
            void BuildCandidateCount();
            void checkCorrectness();
    };
}
