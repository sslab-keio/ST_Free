#include "ST_free.hpp"
#include "inter_analysis.hpp"
#include "statList.hpp"
#include "argList.hpp"
#include "determinator.hpp"
#pragma once

namespace ST_free{
    class Analyzer {
        private:
            static FuncIdentifier identifier;
            static StatusList stat;
            ArgList args;
            Function * Funcs;
        public:
            Analyzer(){
                args = ArgList();
            }
            explicit Analyzer(Function *func){
                Funcs = func;
                args = ArgList(func->arg_size());
            }
            void analyze();
            void analyzeDifferentFunc(Function &);
            void checkStructElements(Instruction *);
            void checkAndMarkFree(Value * V, CallInst *CI);
            void checkAndMarkAlloc(CallInst *CI);
    };
}
