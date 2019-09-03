#include <stdio.h>
#include <stdlib.h>

struct stA{
    int x;
    char *str;
};

int main(int argc, char *argv[])
{
    struct stA * test;
    test = (struct stA *)malloc(sizeof(struct stA));
    test->str = (char *)malloc(sizeof(char) * 8);

    free(test->str);
    free(test);
    
    return 0;
}

void correct(){
    struct stA * test;
    test = (struct stA *)malloc(sizeof(struct stA));
    test->str = (char *)malloc(sizeof(char) * 8);

    free(test->str);
    free(test);
}

void incorrect(){
    struct stA * test;
    test = (struct stA *)malloc(sizeof(struct stA));
    test->str = (char *)malloc(sizeof(char) * 8);

    free(test);
}
