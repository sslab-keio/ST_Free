/*** Intraprocedural / Leaked ***/
#include <stdio.h>
#include <stdlib.h>

struct test{
    int x;
    char name[10];
    struct ref * b;
};

struct ref{
    int y;
    char *str;
};

int main()
{
    struct test* struct_parent;

    struct_parent = (struct test *)malloc(sizeof(struct test));
    struct_parent->b = (struct ref *)malloc(sizeof(struct ref));
    struct_parent->b->str = (char *)malloc(sizeof(char) * 8);
    
    free(struct_parent->b);
    free(struct_parent);

    return 0;
}

