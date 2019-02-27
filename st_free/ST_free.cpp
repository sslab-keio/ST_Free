#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DerivedTypes.h"

// include STL
#include <vector>
#include <map>
#include <queue>
#include <algorithm>

#define NO_ALLOC 0
#define ALLOCATED 1
#define FREED 2

using namespace llvm;
using namespace std;

vector<string> alloc_funcs = {"malloc", "kzalloc", "kmalloc", "zalloc", "vmalloc", "kcalloc"};
vector<string> free_funcs = {"free", "kfree"};

namespace{
    struct status_element{
        Type * struct_type;
        uint64_t index;
        int status;
        status_element(Type * t, uint64_t i){
            struct_type = t;
            index = i;
            status = ALLOCATED;
        }
        friend bool operator==(const status_element &, const status_element &);
    };

    bool operator==(const status_element& a, const status_element& b){
        return a.struct_type == b.struct_type && a.index == b.index;
    }

    vector<status_element> stat_table;

    struct st_free : public FunctionPass {
        static char ID;

        st_free() : FunctionPass(ID){
        }

        /*** Main Moduler ***/
        bool runOnFunction(Function &F) override {
            for (BasicBlock &B: F) {
                for (Instruction &I : B) {
                    if (auto* CI = dyn_cast<CallInst>(&I)) {
                        if (Function* called_function = CI->getCalledFunction()) {
                            if (isAllocFunction(string(called_function->getName()))) {
                                generateWarning(CI, "Found Malloc");
                                if(isStructEleAlloc(CI)){
                                    generateWarning(CI, "Struct element malloc");
                                }
                            } else if (isFreeFunction(string(called_function->getName()))) {
                                for (auto args = CI->arg_begin(); args != CI->arg_end();args++) {
                                    if (Instruction * val = dyn_cast<Instruction>(* args)) {
                                        if (PointerType * ptr_ty = dyn_cast<PointerType>(val->getType())) {
                                            if(isStructEleFree(val)){
                                                generateWarning(val, "Struct element free");
                                            } else if (isStructFree(val)){
                                                checkStructElements(val);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
           return false;
        }

        bool isAllocFunction(string name){
            auto itr = find(alloc_funcs.begin(), alloc_funcs.end(), name);
            if(itr != alloc_funcs.end()){
                return true;
            }
            return false;
        }

        bool isFreeFunction(string name){
            auto itr = find(free_funcs.begin(), free_funcs.end(), name);
            if(itr != free_funcs.end()){
                return true;
            }
            return false;
        }

        /*** Iterate Use until Load Instruction ***/
        LoadInst * find_load(Instruction * val){
            if(isa<LoadInst>(val)){
                return cast<LoadInst>(val);
            }
            for(Use &U : val->operands()){
                if(Instruction * inst = dyn_cast<Instruction>(U)){
                    if(isa<LoadInst>(inst)){
                        return cast<LoadInst>(inst);
                    }
                }
            }
            return NULL;
        }

        bool isStructEleAlloc(Instruction * val){
            for (User *usr: val->users()){
                User * tmp_usr = usr;
                if(!isa<StoreInst>(usr)){
                    for(User * neo_usr: usr->users()){
                        if(isa<StoreInst>(neo_usr)){
                            tmp_usr = neo_usr;
                            break;
                        }
                    }
                }
                if(StoreInst * str_inst = dyn_cast<StoreInst>(tmp_usr)){
                    Value * tgt_op = str_inst->getOperand(1);
                    // outs() << string(tgt_op->getName()) << "\n";
                    if(GetElementPtrInst * inst = dyn_cast<GetElementPtrInst>(tgt_op)){
                        status_element st_ele(inst->getSourceElementType(), cast<ConstantInt>(inst->getOperand(2))->getZExtValue());
                        stat_table.push_back(st_ele);
                        // outs() << *(inst->getSourceElementType()) << "\n";
                        // outs() << cast<ConstantInt>(inst->getOperand(2))->getZExtValue() << "\n";
                        return true;
                    }
                }
            }
            return false;
        }

        bool isStructEleFree(Instruction * val){
            LoadInst * l_inst = find_load(val);
            for(Use &U : l_inst->operands()){
                if(GetElementPtrInst * inst = dyn_cast<GetElementPtrInst>(U)){
                    status_element st_ele(inst->getSourceElementType(), cast<ConstantInt>(inst->getOperand(2))->getZExtValue());
                    auto ele = find(stat_table.begin(), stat_table.end(), st_ele);
                    if(ele != stat_table.end()){
                        generateWarning(l_inst, "Found Allocated Element Free");
                        ele->status = FREED;
                        return true;
                    }
                }
            }
            return false;
        }

        bool isStructFree(Instruction * val){
            LoadInst *load_inst = find_load(val);
            if (load_inst != NULL) {
                Type * tgt_type = get_type(load_inst->getPointerOperand());
                if (tgt_type != NULL && tgt_type->isStructTy()) {
                    generateWarning(load_inst, "Found Struct");
                    return true;
                }
            }
            return false;
        }

        void checkStructElements(Instruction * val){
            int index = 0;
            LoadInst *load_inst = find_load(val);
            StructType * tgt_type = cast<StructType>(get_type(load_inst->getPointerOperand()));
            for(auto ele = tgt_type->element_begin(); ele != tgt_type->element_end(); ele++, index++){
                if((*ele)->isPointerTy()){
                    generateWarning(load_inst, "Has pointer element");
                    status_element st_ele(tgt_type, index);
                    auto ele = find(stat_table.begin(), stat_table.end(), st_ele);
                    if(ele != stat_table.end() && ele->status != FREED){
                        generateWarning(load_inst, "Unfreed pointer element found !!");
                    }
                }
            }
        }

        /*** Retrieve Pointer Dereferance Type ***/
        Type * get_type(Value * val){
            Type * val_type;

            if(val_type == NULL){
                return NULL;
            }

            val_type = val->getType();
            while(val_type->isPointerTy()){
                val_type = (cast<PointerType>(val_type))->getElementType();
            }
            return val_type;
        }

        /*** Check Struct Element for pointer type ***/
        // bool check_struct_ele_ptr(StructType *st_type){
        //     bool has_pointer_ele = false;
        //     for(auto ele = st_type->element_begin(); ele != st_type->element_end(); ele++){
        //         if((*ele)->isPointerTy()){
        //             has_pointer_ele = true;
        //         }
        //     }
        //     return has_pointer_ele;
        // }

        bool isHeapValue(Value *v){
            return true;
        }

        void generateWarning(Instruction * Inst, string warn){
            if(const DebugLoc &Loc = Inst->getDebugLoc()){
                unsigned line = Loc.getLine();
                unsigned col = Loc.getCol();
                outs() << "\033[1;31m[ST_free]\033[0m ";
                // outs() << string(Loc->getDirectory()) << "/" << string(Loc->getFilename()) << ":" << line << ":" << col << ": ";
                outs() << string(Loc->getFilename()) << ":" << line << ":" << col << ": ";
                outs() << warn << "\n";
            }
        }
    }; // end of struct
}  // end of anonymous namespace

char st_free::ID = 0;

static RegisterPass<st_free> X("st_free", "struct free checker",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);

static void registerSTFreePass(const PassManagerBuilder &, legacy::PassManagerBase &PM) {
    PM.add(new st_free());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSTFreePass);

