#include <stdio.h>
#include <stdlib.h>

struct testC {
    int z;
    int hoge;
};

struct testB {
    int x;
    int y;
    struct testC *tC;
};

struct testA {
    int x;
    int y;
    struct testB tB;
};

void freeA(struct testA *tA) {
    free(tA);
}

int main()
{
    struct testA *tA;
    int err;
    tA = (struct testA *)malloc(sizeof(struct testA));
    tA->tB.tC = (struct testC *)malloc(sizeof(struct testC));

    // free(tA->tB.tC);
    // tA->tB.tC = NULL;
    freeA(tA);

    return 0;
}
