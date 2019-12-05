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
    int error;
    tA = (struct testA *)malloc(sizeof(struct testA));
    if (tA == NULL) {
        error = -1;
        goto err1;
    }

    tA->tB = alloctB();
    if (tA->tB == NULL) {
        error = -2;
        goto err2;
    }

    return 0;
err2:
    free(tA);
err1:
    return error;
}

int main()
{
    struct testA *tA;
    int err;
    allocation(tA);
    err = allocation(tA); 

    if (err < 0) {
        return -1;
    }
    // if (allocation(tA) < 0) {
    //     return -1;
    // }
    return 0;
}
