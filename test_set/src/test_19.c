/*** Intraprocedural / Leaked ***/
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

struct test{
    int x;
    char name[10];
    char *str;
};

int main()
{
    struct test* struct_parent;
    char * storedStr;

    struct_parent = (struct test *)malloc(sizeof(struct test));
    struct_parent->str = (char *)malloc(sizeof(char) * 8);
    storedStr = struct_parent->str;
    
    free(struct_parent);

    return 0;
}

