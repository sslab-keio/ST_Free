#include <stdio.h>
#include <stdlib.h>

struct testarr {
    int x;
    struct testA **tA;
};

struct testA {
    int z;
    struct testB *tB;
};

struct testB {
    int y;
};

int main()
{
    struct testarr* ta;
    int index = 0;

    ta = (struct testarr *)malloc(sizeof(struct testarr));
    ta->tA = (struct testA **)malloc(sizeof(struct testA *) * 10);

    for(int i = 0; i < index; i++){
        ta->tA[i] = (struct testA *)malloc(sizeof(struct testA));
        ta->tA[i]->tB = (struct testB *)malloc(sizeof(struct testB));
    }

    do {
      printf("Something");
    } while(index-- > 0);

    for(int i = 0; i < index; i++){
        free(ta->tA[i]->tB);
        free(ta->tA[i]);
    }

    free(ta->tA);
    free(ta);
    return 0;
}
