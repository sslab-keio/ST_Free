#include <stdio.h>
#include <stdlib.h>

struct A {
    int x;
    struct B* b;
};

struct B {
    int y;
};

void freeB(struct B* testB){
    free(testB);
}

int main() {
    struct A* testA;
    struct B* testB;

    testA->b = testB;

    freeB(testB);
    free(testA);

    return 0;
}
