#include <stdio.h>
#include <stdlib.h>

struct test{
    int x;
    char name[10];
    struct ref * b;
};

struct ref{
    int y;
};

int main()
{
    struct test* struct_parent;

    struct_parent = (struct test *)malloc(sizeof(struct test));
    struct_parent->b = (struct ref *)malloc(sizeof(struct ref));

    free(struct_parent);

    return 0;
}

