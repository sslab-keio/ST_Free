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

struct testB* alloctB() {
    struct testB *tB;
    tB = (struct testB *)malloc(sizeof(struct testB));
    return tB;
}

int allocation(struct testA *tA) {
    tA = (struct testA *)malloc(sizeof(struct testA));
    if (tA == NULL) {
        return -1;
    }

    tA->tB = alloctB();
    if (tA->tB == NULL) {
        free(tA);
        return -2;
    }

    return 0;
}

int main()
{
    struct testA *tA;
    int err;

    err = allocation(tA);
    if (err < 0) {
        return -1;
    }
    free(tA);
    return 0;
}
