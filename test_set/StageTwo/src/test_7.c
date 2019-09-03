 /***Intraprocedural / Unleaked ***/

#include <stdio.h>
#include <stdlib.h>

struct test{
    int x;
    char name[10];
    int y;
    int z;
    char * str;
};

void freeElement(unsigned long t);
void freeStructs(void *t);

int main()
{
    struct test* t;

    t = (struct test *)malloc(sizeof(struct test));
    t->x = 10;

    t->str = (char *)malloc(sizeof(char) * 8);

    t->str = "test\0";
    printf("%d, %s",t->x, t->str);

    freeElement((unsigned long) t->str);
    freeStructs(t);
    return 0;
}

void freeElement(unsigned long t){
    free((void *)t);
}

void freeStructs(void *t){
    free(t);
} 
