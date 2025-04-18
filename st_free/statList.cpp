#include "statList.hpp"

using namespace ST_free;

namespace ST_free {
int StatusList::getStat(Type *T, Value *V, uint64_t index) {
  if (this->exists(T, V, index)) {
    return st_tab[T][V][index];
  }
  return NO_ALLOC;
}

void StatusList::setStat(Type *T, Value *V, uint64_t index, int stat) {
  this->setList(T, V);
  if (this->exists(T, V, index)) {
    st_tab[T][V][index] = stat;
  }
  return;
}

bool StatusList::exists(Type *T, Value *V, uint64_t index) {
  if (st_tab.find(T) != st_tab.end()) {
    if (st_tab[T].find(V) != st_tab[T].end()) {
      if (index < st_tab[T][V].size()) return true;
    }
  }
  return false;
}

bool StatusList::exists(Type *T, Value *V) {
  if (st_tab.find(T) != st_tab.end()) {
    if (st_tab[T].find(V) != st_tab[T].end()) {
      return true;
    }
  }
  return false;
}

void StatusList::setList(Type *T, Value *V) {
  if (!this->exists(T, V))
    st_tab[T][V] = vector<int>(cast<StructType>(T)->getNumElements(), NO_ALLOC);
}

vector<int> *StatusList::getList(Type *T, Value *V) {
  if (this->exists(T, V)) return &st_tab[T][V];
  return NULL;
}
size_t StatusList::getSize(Type *T, Value *V) {
  if (this->exists(T, V)) return st_tab[T][V].size();
  return 0;
}
}  // namespace ST_free
