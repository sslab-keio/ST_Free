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

void free_struct(struct test *);
void free_struct_second(struct test * test_struct); 

int main()
{
    struct test* t;

    t = (struct test *)malloc(sizeof(struct test));
    t->b = (struct ref *)malloc(sizeof(struct ref));

    free_struct(t);

    return 0;
}

void free_struct(struct test * test_struct){
    free_struct_second(test_struct);
}

void free_struct_second(struct test * test_struct){
    free(test_struct);
}
