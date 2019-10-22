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
    tA = (struct testA *)malloc(sizeof(struct testA));

    freeA(tA);

    return 0;
}
