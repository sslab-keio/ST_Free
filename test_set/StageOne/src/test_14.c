#include <stdio.h>
#include <stdlib.h>

struct A {
    int x;
    struct B* b;
};

struct B {
    int y;
};

int main()
{
    struct A* testA;
    struct B* testB;
    testA = (struct A *)malloc(sizeof(struct A));
    testB = (struct B *)malloc(sizeof(struct B));

    testA->b = testB;

    free(testB);
    free(testA);

    return 0;
}
