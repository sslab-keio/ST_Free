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
};

void freeA(struct testA *tA) { free(tA); }

int freeB(struct testA *tA) {
  int err;
  if (err < 0) return -1;

  free(tA->tB);
  return 0;
}

int main() {
  struct testA *tA;
  tA = (struct testA *)malloc(sizeof(struct testA));
  tA->tB = (struct testB *)malloc(sizeof(struct testB));

  freeB(tA);
  freeA(tA);

  return 0;
}
