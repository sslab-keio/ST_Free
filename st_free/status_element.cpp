#include "ST_free.hpp"

struct status_element {
  Type* struct_type;
  uint64_t index;
  int status;
  status_element(Type* t, uint64_t i) {
    struct_type = t;
    index = i;
    status = ALLOCATED;
  }
  friend bool operator==(const status_element&, const status_element&);
};

bool operator==(const status_element& a, const status_element& b) {
  return a.struct_type == b.struct_type && a.index == b.index;
}
