#include <stdio.h>
#include <stdlib.h>

struct testA {
    int x;
    char y;
    struct testB * tB;
};

struct testB {
    int ref;
    char * z;
};

int main(int argc, char *argv[])
{
    struct testA * tA;
    tA = (struct testA *)malloc(sizeof(struct testA));
    tA->tB = (struct testB *)malloc(sizeof(struct testB) * 8);

    for (int i  = 0 ; i < 8; i++){
        free(tA->tB[i].z);
    }
    free(tA->tB);
    return 0;
}
