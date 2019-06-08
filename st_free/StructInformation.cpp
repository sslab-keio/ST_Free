#include "include/StructInformation.hpp"

namespace ST_free{
    StructInformation::StructInformation(StructType * st){
        int ind = 0;
        strTy = st;
        memberStats = vector<int>(st->getNumElements(), ISUNKNOWN);
        freedCounts = vector<int>(st->getNumElements(), 0);
        candidateNum = 0;
        for(Type * ty: st->elements()){
            if(!isa<PointerType>(ty))
                memberStats[ind] = NOTPOINTERTY;
            else if(isa<FunctionType>(get_type(ty)))
                memberStats[ind] = ISNOTRESPONSIBLE;
            ind++;
        }
    }
    void StructInformation::BuildCandidateCount(){
        for(CandidateValue* cand : candidates){
            for(unsigned ind = 0; ind < cand->getMemberSize(); ind++){
                if(cand->memberIsFreed(ind) && this->isUnknown(ind)){
                    this->incrementFreedCount(ind);
                }
            }
        }
        return;
    }

    void StructInformation::checkCorrectness(){
        for(CandidateValue* cand : candidates){
            for(unsigned ind = 0; ind < cand->getMemberSize(); ind++){
                if(!cand->memberIsFreed(ind)) {
                    if(this->isResponsible(ind)) {
                        generateError(cand->getInstruction(), "Struct element is NOT Freed");
                        break;
                    } else if(this->isUnknown(ind)){
                        if(this->judgeResponsibility(ind)){
                            generateError(cand->getInstruction(), "Struct element is NOT Freed");
                            break;
                        }
                    }
                }
            }
        }
        return;
    }
    bool StructInformation::judgeResponsibility(int ind){
        int threashold = candidateNum / 2;
        // outs() << ind << " " << threashold << " " << freedCounts[ind] << "\n";
        if(freedCounts[ind] >= threashold)
            return true;
        return false;
    }
    void StructInformation::incrementFreedCount(int ind){
        freedCounts[ind]++;
    }

    void StructInformation::addReferee(StructType * st){
        referees.push_back(st);
    }
    bool StructInformation::hasSingleReferee(){
        return referees.size() == 1;
    }
    void StructInformation::setMemberStatResponsible(int num){
        if(num < memberStats.size())
            memberStats[num] = ISRESPONSIBLE;
    }
    void StructInformation::setMemberStatNotResponsible(int num){
        if(num < memberStats.size())
            memberStats[num] = ISNOTRESPONSIBLE;
    }
    void StructInformation::addCandidateValue(Function *F, FreedStruct *fs){
        candidates.push_back(new CandidateValue(F, fs));
        candidateNum++;
    }
    void StructInformation::print(){
        outs() << "=== StructInfo: Debug Info===\n";
        outs() << "[Struct]: " << *strTy << "\n";
        outs() << "[Elements]\n";
        for(int ind = 0; ind < strTy->getNumElements(); ind++){
            outs() << "\t[Type]: " << *strTy->getElementType(ind) << " ";
            switch(memberStats[ind]){
                case ISRESPONSIBLE:
                    outs() << "ISRESPONSIBLE\n";
                    break;
                case ISNOTRESPONSIBLE:
                    outs() << "ISNOTRESPONSIBLE\n";
                    break;
                case ISUNKNOWN:
                    outs() << "ISUNKNOWN ";
                    if(this->judgeResponsibility(ind))
                        outs() << "(Judged Responsible)\n";
                    else
                        outs() << "(Judged Not Responsible)\n";
                    break;
                case NOTPOINTERTY:
                    outs() << "NOTPOINTERTY\n";
                    break;
                default:
                    outs() << "DEFAULT\n";
            }
        }
        outs() << "[CandidateNum]: " << candidateNum << "\n";
        outs() << "[BugCandidates]\n";
        if(candidates.size() == 0)
            outs() << "Empty\n";
        for(CandidateValue* cands: candidates){
            outs() << "\t[Function] " << cands->getFunction()->getName() << "\n";
            outs() << "\t[FreedStruct] " << cands->getFreedStruct()->getValue()->getName() << "\n";
            cands->print();
            outs() << "******\n";
        }
        outs() << "=============================\n";
    }
    

    /*** [Struct Manager] ***/
    StructManager::StructManager(vector<StructType *> st){
        for(StructType * StrTy:st)
            StructInfo[StrTy] = new StructInformation(StrTy);
        this->createDependencies();
        this->changeStats();
    }

    void StructManager::addReferee(StructType *tgt, StructType *referee){
        StructInfo[tgt]->addReferee(referee);
    }

    void StructManager::createDependencies(){
        for(auto Stmap : StructInfo){
            for(Type * member : Stmap.first->elements()){
                if(member->isPointerTy())
                    if(auto stTy = dyn_cast<StructType>(get_type(member)))
                        this->addReferee(stTy, Stmap.first);
            }
        }
    }
    void StructManager::changeStats(){
        for(auto Stmap : StructInfo){
            for(unsigned ind = 0; ind < Stmap.first->getNumElements(); ind++){
                Type * member = Stmap.first->getElementType(ind);
                if(member->isPointerTy())
                    if(auto stTy = dyn_cast<StructType>(get_type(member)))
                        if(StructInfo[stTy]->hasSingleReferee()){
                            StructInfo[Stmap.first]->setMemberStatResponsible(ind);
                        }
            }
        }
    }

    void StructManager::addCandidateValue(Function *F, StructType *strTy, FreedStruct * fs){
        StructInfo[strTy]->addCandidateValue(F, fs);
    }

    void StructManager::print(){
        for(auto Stmap : StructInfo){
            Stmap.second->print();
        }
    }

    void StructManager::BuildCandidateCount(){
        for(auto Stmap : StructInfo){
            Stmap.second->BuildCandidateCount();
        }
    }
    void StructManager::checkCorrectness(){
        for(auto Stmap : StructInfo){
            Stmap.second->checkCorrectness();
        }
    }
}
