#include <stdio.h>
#include <stdlib.h>

struct test{
    int x;
    struct ref * b;
};

struct ref{
    int y;
};

int main()
{
    struct test* t;

    t = (struct test *)malloc(sizeof(struct test));
    t->b = (struct ref *)malloc(sizeof(struct ref));

    free(t);

    return 0;
}

