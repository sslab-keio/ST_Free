#include "ST_free.hpp"
#include "inter_analysis.hpp"
#include "statList.hpp"
#pragma once

namespace ST_free{
    class Analyzer {
        private:
            static FuncIdentifier identifier;
            static StatusList stat;
            vector<Value *> arg_list;
            // Function F;
        public:
            // Analyzer(Function func){
            //     F = func;
            // };
            void analyze(Function &);
            void analyzeDifferentFunc(Function &);
            void checkStructElements(Instruction *);
    };
}
