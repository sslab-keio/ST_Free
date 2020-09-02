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

void alloctB(struct testB** tB) {
    struct testB *tmp;
    tmp = (struct testB *)malloc(sizeof(struct testB));
    *tB = tmp;
}

int main()
{
    struct testA *tA;
    struct testB *tB;

    tA = (struct testA *)malloc(sizeof(struct testA));
    alloctB(&tB);
    tA->tB = tB;

    free(tA);
    return 0;
}
