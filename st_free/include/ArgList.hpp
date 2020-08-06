#pragma once
#include "ST_free.hpp"
#include "support_funcs.hpp"

namespace ST_free {
/**
 * [Class] ArgStatus
 * Keeps Track of each function arguments, and its stats.
 * If the Argument is StructType, and modifies the member of that argument,
 * it add a new ArgStatus Instance to structStatus vector.
 */
class ArgStatus {
 public:
  ArgStatus(uint64_t structSize) {
    freed = false;
    structStatus = vector<ArgStatus>(structSize);
    statusSize = structSize;
    T = NULL;
  }
  ArgStatus(Type *Ty) {
    freed = false;
    T = Ty;
    structStatus = vector<ArgStatus>();
    statusSize = 0;
  }
  ArgStatus() {
    freed = false;
    T = NULL;
    structStatus = vector<ArgStatus>();
    statusSize = 0;
  }
  bool isFreed() { return freed; }
  void setFreed() { freed = true; }
  void setType(Type *Ty) { T = Ty; }
  Type *getType() { return T; }
  bool isStruct() { return T && get_type(T)->isStructTy(); }
  uint size() { return statusSize; }
  int maxSize();
  bool isMemberFreed(queue<int> indexes);
  void setMemberFreed(queue<int> indexes);
  void extendStatusSize(int extSize);
  ArgStatus *getStatus(int index) {
    return index >= 0 && index < this->size() ? &structStatus[index] : NULL;
  }
  vector<bool> getFreedList();

 private:
  bool freed;
  vector<ArgStatus> structStatus;
  Type *T;
  int statusSize;
};
class ArgList {
 public:
  ArgList() { arg_list = vector<Value *>(); }
  explicit ArgList(uint64_t arg_num) {
    argNum = arg_num;
    arg_list = vector<Value *>(arg_num, NULL);
    stats = vector<ArgStatus>(arg_num);
  }
  void setArg(uint64_t arg_no, Value *V);
  Value *getArg(uint64_t arg_no);
  ArgStatus *getArgStatus(int arg) {
    return (arg >= 0 && arg < stats.size()) ? &stats[arg] : NULL;
  };
  void setArgs(Function *F);
  bool isInList(Value *V);
  int64_t getOperandNum(Value *V);
  void setFreed(Value *V);
  void setFreed(Value *V, queue<int> indexes);
  bool isFreed(Value *V, queue<int> indexes);

 private:
  uint64_t argNum;
  vector<Value *> arg_list;
  vector<ArgStatus> stats;
};
}  // namespace ST_free
