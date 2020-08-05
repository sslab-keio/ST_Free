/*** Interprocedural / Leaked ***/

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

void free_struct(struct test *, int x);

int main()
{
    struct test* t;
    int x;

    t = (struct test *)malloc(sizeof(struct test));
    t->b = (struct ref *)malloc(sizeof(struct ref));

    free_struct(t, x);

    return 0;
}

void free_struct(struct test * test_struct, int x){
    free(test_struct);
}
