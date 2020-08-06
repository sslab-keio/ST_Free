#include "ST_free.hpp"
#pragma once

struct status_element {
  llvm::Type *struct_type;
  uint64_t index;
  int status;
  status_element(llvm::Type *t, uint64_t i) {
    struct_type = t;
    index = i;
    status = ALLOCATED;
  }
  friend bool operator==(const status_element &, const status_element &);
};
