#include <stdio.h>
#include <stdlib.h>

struct testA {
  int x;
  int y;
  struct testB *tB;
};

struct testB {
  int x;
  int y;
  int z;
  struct testC *tC;
};

struct testC {
  int x;
  struct testD *tD;
};

struct testD {
  int x;
  int y;
  int z;
  int p;
};

struct testB *alloctB() {
  struct testB *tB;
  tB = (struct testB *)malloc(sizeof(struct testB));
  return tB;
}

struct testB *allocation(struct testA *tA) {
  struct testB *tB = (struct testB *)malloc(sizeof(struct testB));
  if (tB == NULL) return NULL;

  struct testC *tC = (struct testC *)malloc(sizeof(struct testC));
  if (tC == NULL) {
    free(tB);
    return NULL;
  } else {
    tB->tC = tC;
  }

  tA->tB = tB;
  return tB;
}

int main() {
  struct testA *tA;

  tA = (struct testA *)malloc(sizeof(struct testA));

  if (!allocation(tA)) {
    free(tA);
    return -1;
  }
  return 0;
}
