#include "ST_free.hpp"
#pragma once

namespace ST_free {
class LoopManager {
 private:
  map<Function *, LoopInfo *> loopinfo;

 public:
  LoopManager(){};
  void add(Function *F, LoopInfo *li);
  LoopInfo *get(Function *F);
  bool exists(Function *F);
};
}  // namespace ST_free
