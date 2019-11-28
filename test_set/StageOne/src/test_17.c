#include <stdio.h>
#include <stdlib.h>

struct A {
    int x;
    struct B * testB;
    struct ops *ops;
};

struct ops {
    int (*releaseB)(struct B *testB);
};

struct B {
    int z;
};

int release(struct B * testB);

struct ops ops = {
    .releaseB = &release,
};

int main(){
    struct A *testA;
    testA = (struct A *)malloc(sizeof(struct A));
    testA->testB = (struct B *)malloc(sizeof(struct B));
    testA->ops = &ops;

    testA->ops->releaseB(testA->testB);
    free(testA);

    return 0;
}

int release (struct B* testB) {
    free(testB);
    return 0;
}
