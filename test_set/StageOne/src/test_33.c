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

int main()
{
    struct testA *tA;
    struct testC *tC;
    tA = (struct testA *)malloc(sizeof(struct testA));
    tC = (struct testC *)malloc(sizeof(struct testC));
    tC->tD = (struct testD *)malloc(sizeof(struct testD));
    tA->tB = (struct testB *) tC->tD;

    free(tA);
    return 0;
}
