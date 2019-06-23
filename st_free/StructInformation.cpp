#include "include/StructInformation.hpp"

namespace ST_free{
    StructInformation::StructInformation(StructType * st){
        int ind = 0;
        strTy = st;
        memberStats = vector<int>(st->getNumElements(), ISUNKNOWN);
        freedCounts = vector<int>(st->getNumElements(), 0);
        stc = vector<storeCount>(st->getNumElements());
        candidateNum = 0;
        allocNum = 0;
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
                // if(cand->memberIsFreed(ind) && this->isUnknown(ind)){
                if(cand->memberIsFreed(ind)){
                    this->incrementFreedCount(ind);
                }
            }
        }
        return;
    }

    void StructInformation::checkCorrectness(){
        for(CandidateValue* cand : candidates){
            string warningStr("Struct element is NOT Freed (indexes: ");
            bool hasWarning = false;
            for(unsigned ind = 0; ind < cand->getMemberSize(); ind++){
                if(!cand->memberIsFreed(ind)) {
                    if(this->isResponsible(ind)) {
                        if(this->judgeResponsibility(ind) && !this->isAllStoreGlobalVar(ind)){
                            // generateError(cand->getInstruction(), "Struct element is NOT Freed");
                            warningStr += to_string(ind);
                            warningStr += ' ';
                            hasWarning = true;
                        }
                        // cand->print();
                        // break;
                    } else if(this->isUnknown(ind)){
                        if(this->judgeResponsibility(ind)){
                            // generateError(cand->getInstruction(), "Struct element is NOT Freed");
                            // break;
                        }
                    }
                }
            }
            warningStr += ')';
            if(hasWarning){
                generateError(cand->getInstruction(), warningStr);
                // this->print();
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

    void StructInformation::setMemberStatNotAllocated(int num){
        if(num < memberStats.size())
            memberStats[num] = UNALLOCATED;
    }

    void StructInformation::addCandidateValue(Function *F, FreedStruct *fs){
        candidates.push_back(new CandidateValue(F, fs));
        candidateNum++;
    }

    void StructInformation::incrementStoreTotal(int ind){
        if(ind < stc.size())
            stc[ind].total++;
    }

    void StructInformation::incrementStoreGlobalVar(int ind){
        if(ind < stc.size())
            stc[ind].globalVar++;
    }

    void StructInformation::print(){
        outs() << "=== StructInfo: Debug Info===\n";
        outs() << "[Struct]: " << strTy->getName() << "\n";
        outs() << "[AllocNum]: " << allocNum << "\n";
        outs() << "[Referees] (TTL: " << referees.size() << ") \n";
        // for (StructType* ty : referees){
        //     outs() << "\t" << *ty << "\n";
        // }
        outs() << "[Elements]\n";
        for(int ind = 0; ind < strTy->getNumElements(); ind++){
            outs() << "\t[Type]: " << *strTy->getElementType(ind) << " ";
            switch(memberStats[ind]){
                case ISRESPONSIBLE:
                    outs() << "ISRESPONSIBLE";
                    break;
                case ISNOTRESPONSIBLE:
                    outs() << "ISNOTRESPONSIBLE";
                    break;
                case ISUNKNOWN:
                    outs() << "ISUNKNOWN ";
                    if(this->judgeResponsibility(ind))
                        outs() << "(Judged Responsible)";
                    else
                        outs() << "(Judged Not Responsible)";
                    break;
                case NOTPOINTERTY:
                    outs() << "NOTPOINTERTY";
                    break;
                case UNALLOCATED:
                    outs() << "UNALLOCATED";
                    break;
                default:
                    outs() << "DEFAULT";
            }
            outs() << "[" << stc[ind].globalVar << "/" << stc[ind].total << "]\n";
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
        if(!this->exists(tgt))
            StructInfo[tgt] = new StructInformation(tgt);
        StructInfo[tgt]->addReferee(referee);
    }

    bool StructManager::exists(StructType * st){
        if(StructInfo.find(st) != StructInfo.end())
            return true;
        return false;
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
        if(!this->exists(strTy)){
            StructInfo[strTy] = new StructInformation(strTy);
        }
        StructInfo[strTy]->addCandidateValue(F, fs);
    }

    void StructManager::addAlloc(StructType *strTy){
        if(!this->exists(strTy)){
            StructInfo[strTy] = new StructInformation(strTy);
        }
        StructInfo[strTy]->incrementAllocNum();
    }

    void StructManager::addStore(StructType *strTy, int ind){
        if(!this->exists(strTy)){
            StructInfo[strTy] = new StructInformation(strTy);
        }
        StructInfo[strTy]->incrementStoreTotal(ind);
    }

    void StructManager::addGlobalVarStore(StructType *strTy, int ind){
        if(!this->exists(strTy)){
            StructInfo[strTy] = new StructInformation(strTy);
        }
        StructInfo[strTy]->incrementStoreGlobalVar(ind);
    }

    void StructManager::print(){
        for(auto Stmap : StructInfo){
            Stmap.second->print();
        }
    }

    void StructManager::BuildCandidateCount(){
        // this->checkNonAllocs();
        for(auto Stmap : StructInfo){
            Stmap.second->BuildCandidateCount();
        }
    }

    void StructManager::checkCorrectness(){
        for(auto Stmap : StructInfo){
            Stmap.second->checkCorrectness();
        }
    }
     
    void StructManager::checkNonAllocs(){
        for(auto Stmap : StructInfo){
            int ind = 0;
            for(Type * member : Stmap.first->elements()){
                if(member->isPointerTy())
                    if(auto stTy = dyn_cast<StructType>(get_type(member))) {
                        if(StructInfo[stTy]->hasNoAlloc()){
                            Stmap.second->setMemberStatNotAllocated(ind);
                        }
                    }
                ind++;
            }
        }
    }
}
