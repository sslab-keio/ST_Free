/*** Intraprocedural / Unleaked ***/

#include <stdio.h>
#include <stdlib.h>

struct test{
    int x;
    char name[10];
    char * str;
};

int main()
{
    struct test* t;

    t = (struct test *)malloc(sizeof(struct test));
    t->str = (char *)malloc(sizeof(char) * 8);

    free(t->str);
    free(t);

    return 0;
}

