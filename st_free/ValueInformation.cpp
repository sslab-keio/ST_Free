#include "include/ValueInformation.hpp"
namespace ST_free{
    Value * ValueInformation::getValue() const{
        return V;
    }
    Type * ValueInformation::getStructType() const{
        return structType;
    }

    Type * ValueInformation::getMemberType() const{
        return memberType;
    }

    long ValueInformation::getMemberNum() const{
        return memberNum;
    }

    bool ValueInformation::isStructMember(){
        if (memberType == NULL && structType == NULL)
            return false;
        return true;
    }
}
