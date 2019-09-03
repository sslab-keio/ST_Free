#include <stdio.h>
#include <stdlib.h>

struct testA {
    int x;
    char y;
    const struct testA_ops *ops;
};

struct testA_ops {
    int z;
    int k;
};

const static struct testA_ops tao = {
    .z = 1,
    .k = 2,
};

int main()
{
    struct testA * tsta;
    tsta = (struct testA *)malloc(sizeof(struct testA));
    tsta->ops = &tao;

    free(tsta);
    return 0;
}
