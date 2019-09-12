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

void freeC(struct C* c){
    free(c);
}

int main()
{
    struct A* testA;

    // freeC(testA->b->c);
    // freeB(testA);
    free(testA);

    return 0;
}
