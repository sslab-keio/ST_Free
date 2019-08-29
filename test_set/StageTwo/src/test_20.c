#include <stdio.h>
#include <stdlib.h>

struct testA {
    int x;
    int y;
    char * str;
};

int main(int argc, char *argv[])
{
    struct testA * tst;

    tst = (struct testA *)malloc(sizeof(struct testA));
    if(tst->str)
        free(tst->str);
    if(tst->str != NULL)
        free(tst->str);
    free(tst);
    return 0;
}
