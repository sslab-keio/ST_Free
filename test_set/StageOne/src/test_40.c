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
    struct testD *tD;
};

struct testD {
    int x;
    int y;
    int z;
    int p;
};

struct testB* alloctB() {
    struct testB *tB;
    tB = (struct testB *)malloc(sizeof(struct testB));
    return tB;
}

struct testA* allocation() {
    struct testA *tA = (struct testA *)malloc(sizeof(struct testA));
    if (tA == NULL) 
        goto err;

    struct testB *tB = (struct testB *)malloc(sizeof(struct testB));
    // tA->tB = alloctB();
    if (tB == NULL)
        goto err2;

    tA->tB = tB;

    struct testC *tC = (struct testC *)malloc(sizeof(struct testC));
    if (tC == NULL)
        goto err3;

    tA->tB->tC = tC;
    return tA;

err3:
    free(tB);
err2:
    free(tA);
err:
    return NULL;
}

int main()
{
    struct testA *tA;

    tA = allocation(); 
    if (tA == NULL)
        return -1;

    return 0;
}
