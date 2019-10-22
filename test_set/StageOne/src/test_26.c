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

    if (tA->tB)
        return -1;

    free(tA->tB);
    freeA(tA);

    return 0;
}
