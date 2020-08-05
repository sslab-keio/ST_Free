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

struct testA* __alocate_a() {
  struct testA* tA = (struct testA *)malloc(sizeof(struct testA));
  if (tA == NULL)
    return NULL;
  return tA;
}

int allocate_a(struct testA* tA) {
  struct testA* tempA;
  tempA = __alocate_a();
  if (!tempA) {
    return -1;
  }
  tA = tempA;
  return 0;
}

int main() {
  struct testA *tA;
  struct testB *tB;
  int err;

  err = allocate_a(tA);
  if (err)
    goto err;

  tB = (struct testB *)malloc(sizeof(struct testB));
  if (tB == NULL)
    goto err2;

  tA->tB = tB;

  // Do something here
  printf("In success block\n");

err3:
  free(tB);
err2:
  free(tA);
err:
  return 0;
}
