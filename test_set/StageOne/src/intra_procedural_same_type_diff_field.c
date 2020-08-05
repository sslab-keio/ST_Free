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
    struct testB tB;
};

int main()
{
    struct testA *tA;
    struct testC *tC;
    tA = (struct testA *)malloc(sizeof(struct testA));
    tA->tB = (struct testB *)malloc(sizeof(struct testB));
    tC = (struct testC *)malloc(sizeof(struct testC));

    free(tC);
    free(tA);

    return 0;
}
