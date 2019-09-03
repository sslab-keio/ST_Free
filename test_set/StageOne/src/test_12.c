/*** Interprocedural / Unleaked ***/
#include <stdio.h>
#include <stdlib.h>

struct test{
    int x;
    char name[10];
    struct ref * b;
};

struct ref{
    int y;
    struct test *parent;
};

void free_struct(struct ref *);

int main()
{
    struct test* t;

    t = (struct test *)malloc(sizeof(struct test));
    t->b = (struct ref *)malloc(sizeof(struct ref));
    t->b->parent = t;

    free_struct(t->b);
    free(t);

    return 0;
}

void free_struct(struct ref * test_struct){
    free(test_struct);
}
