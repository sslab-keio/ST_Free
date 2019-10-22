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
    int y;
    int z;
    int a;
    struct testD *tD;
};

struct testD {
    int x;
    int y;
    int z;
    int a;
    int b;
};

void freetD(struct testB * tB) {
    free(tB->tC->tD);
}

int main()
{
    struct testA *tA;
    tA = (struct testA *)malloc(sizeof(struct testA));
    tA->tB = (struct testB *)malloc(sizeof(struct testB));
    tA->tB->tC = (struct testC *)malloc(sizeof(struct testC));
    tA->tB->tC->tD = (struct testD *)malloc(sizeof(struct testD));

    free(tA->tB->tC->tD);
    free(tA->tB->tC);
    free(tA->tB);
    free(tA);

    return 0;
}
