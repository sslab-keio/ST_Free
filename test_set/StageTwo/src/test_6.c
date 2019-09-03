/*** Interprocedural / Unleaked ***/
#include <stdio.h>
#include <stdlib.h>

struct test{
    int x;
    char name[10];
    struct ref * b;
    int *y;
    int *z;
};

struct ref{
    int y;
};

void free_struct(void *);

int main()
{
    struct test* t;

    t = (struct test *)malloc(sizeof(struct test));
    t->b = (struct ref *)malloc(sizeof(struct ref));
    t->y = (int *)malloc(sizeof(int));
    t->z = (int *)malloc(sizeof(int));

    free(t->y);
    free(t->z);
    free_struct(t->b);
    free(t);

    return 0;
}

void free_struct(void * test_struct){
    free(test_struct);
}
