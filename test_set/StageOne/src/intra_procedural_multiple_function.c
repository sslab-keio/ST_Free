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
    tA = (struct testA *)malloc(sizeof(struct testA));

    free(tA);
    return 0;
}

int alloctB() {
    struct testB *tB;
    tB = (struct testB *)malloc(sizeof(struct testB));
    return 0;
}
