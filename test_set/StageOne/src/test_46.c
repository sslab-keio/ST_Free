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

void free_all(struct testA *tA) {
  if (tA->tB)
    free(tA->tB);
  free(tA);
}

int main() {
  struct testA* tA = (struct testA *)malloc(sizeof(struct testA));
  if (tA == NULL)
    return -1;

  struct testB *tB = (struct testB *)malloc(sizeof(struct testB));
  if (tB == NULL)
    return -1;
  tA->tB = tB;

  free_all(tA);
  return 0;
}
