#include <stdio.h>
#include <stdlib.h>

struct A {
    int x;
    struct B* b;
};

struct B {
    int y;
    struct C* c;
};

struct C{
    int z;
};

void freeB(struct A* a){
    free(a->b);
}

void freeC(struct B* b){
    free(b->c);
}

int main()
{
    struct A* testA;
    testA = (struct A *)malloc(sizeof(struct A));
    testA->b = (struct B *)malloc(sizeof(struct B));
    testA->b->c = (struct C *)malloc(sizeof(struct C));

    freeC(testA->b);
    freeB(testA);
    free(testA);

    return 0;
}
