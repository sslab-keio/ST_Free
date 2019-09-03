#include<stdio.h>
#include<stdlib.h>

struct page_walk {
    struct page_walk * next;
    int x;
    int y;
};

int main(int argc, char *argv[])
{
    struct page_walk *pgwlk, *tmp;
    pgwlk = (struct page_walk *)malloc(sizeof(struct page_walk));

    tmp = pgwlk;

    while(tmp){
		struct page_walk *pos = tmp;
		tmp = tmp->next;
		free(pos);
    }
    free(pgwlk);
    return 0;
}
