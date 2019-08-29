#include <stdio.h>
#include <stdlib.h>

struct test {
    int x;
    int y;
    char * str;
};

void freeStruct(struct test *t);

int main(){
    struct test *t;
    t = (struct test *)malloc(sizeof(struct test));
    t->str = (char *)malloc(sizeof(char) * 8);
    
    freeStruct(t);
    return 0;
}

void freeStruct(struct test *t){
    free(t->str);
    free(t);
}
