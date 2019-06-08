#pragma once
#include "ST_free.hpp"
#include "determinator.hpp"
#include "support_funcs.hpp"
#include "functionManager.hpp"

#define ISRESPONSIBLE 1
#define ISNOTRESPONSIBLE 2
#define ISUNKNOWN 3
#define NOTPOINTERTY 4

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
    class StructInformation {
        private:
            StructType * strTy;
            vector<StructType *> referees;
            vector<int> memberStats;
            vector<int> freedCounts;
            int candidateNum;
            vector<CandidateValue *> candidates;
            bool isResponsible(int ind){return memberStats[ind] == ISRESPONSIBLE;};
            bool isUnknown(int ind){return memberStats[ind] == ISUNKNOWN;};
            bool judgeResponsibility(int ind);
        public:
            StructInformation(){};
            StructInformation(StructType * st);
            void addReferee(StructType * st);
            bool hasSingleReferee();
            void setMemberStatResponsible(int num);
            void setMemberStatNotResponsible(int num);
            void addCandidateValue(Function *F, FreedStruct *fs);
            void print();
            void BuildCandidateCount();
            void checkCorrectness();
            vector<CandidateValue *> getCandidateValue() const{return candidates;};
            void incrementFreedCount(int ind);
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
        public:
            StructManager(){};
            StructManager(vector<StructType *> st);
            StructInformation * get(StructType * strTy){return StructInfo[strTy];}
            void addCandidateValue(Function *F, StructType *strTy, FreedStruct * fs);
            void print();
            void BuildCandidateCount();
            void checkCorrectness();
    };
}
