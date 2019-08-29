#include <stdio.h>
#include <stdlib.h>

struct shared{
    int x;
    int z;
};

struct A{
    int x;
    char z;
    struct shared * stshared;
};
struct B {
    int x;
    char z;
    struct shared * stshared;
};

int main(){
    struct A * testA;
    testA = (struct A *)malloc(sizeof(struct A));

    return 0;
}
