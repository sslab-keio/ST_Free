#include <stdio.h>
#include <stdlib.h>

struct testA{
    int refCount;
    char* str;
};

struct testStruct {
    int x;
    struct testA * A;
};

int main(int argc, char *argv[])
{
    struct testStruct * ts;
    ts = (struct testStruct *)malloc(sizeof(struct testStruct));
    ts->A = (struct testA *)malloc(sizeof(struct testA));
    (ts->A)->refCount++;

    (ts->A)->refCount--;
    if((ts->A)->refCount == 0)
        free(ts->A);
    free(ts);
    
    return 0;
}
