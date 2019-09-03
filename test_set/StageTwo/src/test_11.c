#include <stdio.h>
#include <stdlib.h>


struct test2 {
    int * z;
    int log;
    char * str;
};

struct test {
    struct test2 t2;
    int x;
    int count;
};

int main(int argc, char *argv[])
{
    struct test t;
    t.t2.z = (int *)malloc(sizeof(int));
    t.t2.str = (char *)malloc(sizeof(char) * 8);

    return 0;
}
