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

int some_function_returning_err() {
  int err;
  if (err)
    return -1;
  return 0;
}

int main() {
  struct testA *tA;
  struct testB *tB;

  tA = (struct testA *)malloc(sizeof(struct testA));
  if (tA == NULL)
    goto err;

  tB = (struct testB *)malloc(sizeof(struct testB));
  if (tB == NULL)
    goto err2;

  tA->tB = tB;

  if (some_function_returning_err()) {
    goto err3;
  }

  // Do something here
  printf("In success block\n");

err3:
  free(tB);
err2:
  free(tA);
err:
  return 0;
}
