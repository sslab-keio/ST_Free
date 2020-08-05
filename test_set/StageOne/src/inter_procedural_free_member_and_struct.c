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
    int z;
    struct testD *tD;
};

struct testD {
    int p;
};

void freeA(struct testA *tA) {
    free(tA);
}

int freeB(struct testB *tB) {
    free(tB);

    free(tB->tC);
    return 0;
}

int main()
{
    struct testA *tA;
    tA = (struct testA *)malloc(sizeof(struct testA));
    tA->tB = (struct testB *)malloc(sizeof(struct testB));
    tA->tB->tC = (struct testC *)malloc(sizeof(struct testC));
    tA->tB->tC->tD = (struct testD *)malloc(sizeof(struct testD));

    freeB(tA->tB);
    freeA(tA);

    return 0;
}
