#include <stdio.h>
#include <stdlib.h>

struct testC {
    int z;
    int hoge;
    char * ptr;
};

struct testB {
    int x;
    int y;
    struct testC **tC;
};

struct testA {
    int x;
    int y;
    struct testB *tB;
};

void freeA(struct testA *tA) {
    free(tA);
}

int main()
{
    struct testA *tA;
    int err;
    tA = (struct testA *)malloc(sizeof(struct testA));
    tA->tB = (struct testB *)malloc(sizeof(struct testB));
    tA->tB->tC = (struct testC **)malloc(sizeof(struct testC *) * 10);

    free(tA->tB->tC);
    free(tA->tB);
    freeA(tA);

    return 0;
}
