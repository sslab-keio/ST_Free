#include <stdio.h>
#include <stdlib.h>

struct testB {
    int z;
};

struct testA {
    struct testB *B;
    int x;
};

void allocate_and_store(struct testA* tA, struct testB* tB) {
    tA->B = tB;
}

int main()
{
    struct testA *tA;
    struct testB *tB;

    tA = (struct testA*)malloc(sizeof(struct testA)); 
    if (tA == NULL)
        return -1;

    tB = (struct testB*)malloc(sizeof(struct testB)); 

    allocate_and_store(tA, tB);
    free(tA);

    return 0;
}
