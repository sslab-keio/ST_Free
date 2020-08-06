#include "include/support_funcs.hpp"

using namespace ST_free;

namespace ST_free {
/*** Iterate Use until Load Instruction ***/
llvm::LoadInst* find_load(llvm::Instruction* val) { return find_load_recursively(val, 2); }

static llvm::LoadInst* find_load_recursively(llvm::Instruction* I, int TTL) {
  if (TTL < 0) return NULL;

  if (llvm::isa<llvm::LoadInst>(I))
    return llvm::cast<llvm::LoadInst>(I);
  else if (llvm::isa<llvm::CallInst>(I))
    return NULL;
  // else if(isa<BitCastInst>(I))
  //     return NULL;

  for (llvm::Use& U : I->operands()) {
    if (llvm::Instruction* inst = llvm::dyn_cast<llvm::Instruction>(U)) {
      llvm::LoadInst* res = find_load_recursively(inst, TTL - 1);
      if (res) return res;
    }
  }
  return NULL;
}

llvm::Value* getLoadeeValue(llvm::Value* val) {
  llvm::Value* v = val;
  while (llvm::isa<llvm::LoadInst>(v) || llvm::isa<llvm::GetElementPtrInst>(v)) {
    if (auto inst = llvm::dyn_cast<llvm::LoadInst>(v))
      v = inst->getPointerOperand();
    else if (auto inst = llvm::dyn_cast<llvm::GetElementPtrInst>(v))
      v = inst->getPointerOperand();
  }
  return v;
}
/*** Retrieve Pointer Dereferance Type ***/
llvm::Type* get_type(llvm::Value* val) {
  llvm::Type* val_type = NULL;

  if (val == NULL) return NULL;

  if (auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(val)) {
    val_type = allocaInst->getAllocatedType();
    if (val_type->isPointerTy())
      val_type = (llvm::cast<llvm::PointerType>(val_type))->getElementType();
  } else if (auto GEleInst = llvm::dyn_cast<llvm::GetElementPtrInst>(val)) {
    val_type = GEleInst->getSourceElementType();
    if (val_type->isPointerTy())
      val_type = (llvm::cast<llvm::PointerType>(val_type))->getElementType();
  }

  return val_type;
}

llvm::Type* get_type(llvm::Type* t) {
  llvm::Type* val_type = NULL;

  if (t == NULL) return NULL;

  val_type = t;
  if (val_type->isPointerTy())
    val_type = (llvm::cast<llvm::PointerType>(val_type))->getElementType();
  else if (val_type->isArrayTy())
    val_type = (llvm::cast<llvm::ArrayType>(val_type))->getElementType();
  return val_type;
}

void generateWarning(llvm::Instruction* Inst, string warn, bool print) {
  if (const llvm::DebugLoc& Loc = Inst->getDebugLoc()) {
    unsigned line = Loc.getLine();
    unsigned col = Loc.getCol();
    if (print) {
      llvm::outs() << "[WARNING] ";
      llvm::outs() << string(Loc->getFilename()) << ":" << line << ":" << col << ": ";
      llvm::outs() << warn << "\n";
    }
  }
}

void generateWarning(llvm::Instruction* Inst, llvm::Value* val) {
  if (const llvm::DebugLoc& Loc = Inst->getDebugLoc()) {
    unsigned line = Loc.getLine();
    unsigned col = Loc.getCol();
    llvm::outs() << "[WARNING] ";
    llvm::outs() << string(Loc->getFilename()) << ":" << line << ":" << col << ": ";
    llvm::outs() << *val << "\n";
  }
}

void generateWarning(string warn) {
  DEBUG_WITH_TYPE("st_free", llvm::outs() << "[WARNING] ");
  DEBUG_WITH_TYPE("st_free", llvm::outs() << warn << "\n");
}

void generateError(llvm::Instruction* Inst, string warn) {
  if (const llvm::DebugLoc& Loc = Inst->getDebugLoc()) {
    unsigned line = Loc.getLine();
    unsigned col = Loc.getCol();
    llvm::outs() << "[ERROR] ";
    llvm::outs() << string(Loc->getFilename()) << ":" << line << ":" << col << ": ";
    llvm::outs() << warn << "\n";
  }
}

llvm::Value* getArgAlloca(llvm::Value* arg) {
  for (auto usr = arg->user_begin(); usr != arg->user_end(); usr++) {
    if (llvm::StoreInst* str_inst = llvm::dyn_cast<llvm::StoreInst>(*usr)) {
      return str_inst->getOperand(1);
    }
  }
  return NULL;
}
string parseErrorMessage(llvm::StructType* parent, long index) {
  string message = "";

  message += "Parent: ";
  if (parent && parent->hasName())
    message += parent->getName();
  else
    message += "Unavailable";

  message += " index:";
  message += to_string(index);
  return message;
}
}  // namespace ST_free
