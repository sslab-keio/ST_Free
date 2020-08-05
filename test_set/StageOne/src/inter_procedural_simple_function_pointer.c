#include <stdio.h>
#include <stdlib.h>

struct A {
    int x;
    struct B *testB;
};

struct B {
    int z;
};

int release(struct B * testB);

int main(){
    struct A *testA;
    int (*releaseB)(struct B*);
    testA = (struct A *)malloc(sizeof(struct A));
    testA->testB = (struct B *)malloc(sizeof(struct B));

    releaseB = release;

    (*releaseB)(testA->testB);
    free(testA);

    return 0;
}

int release (struct B* testB) {
    free(testB);
    return 0;
}
