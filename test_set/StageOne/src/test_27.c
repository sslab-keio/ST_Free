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

void freeA(struct testA *tA) {
    free(tA);
}

int main()
{
    struct testA *tA;
    int err;
    tA = (struct testA *)malloc(sizeof(struct testA));
    tA->tB = (struct testB *)malloc(sizeof(struct testB));

    if (err < 0)
        goto freeStruct;

    free(tA->tB);

freeStruct:
    freeA(tA);
    return 0;
}
