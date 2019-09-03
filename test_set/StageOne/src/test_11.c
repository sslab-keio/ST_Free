/*** Intraprocedural / Leaked ***/
#include <stdio.h>
#include <stdlib.h>

struct node{
    int x;
    char * str;
    struct node * next;
};

int main(){
    struct node * head, * next_node;
    head = (struct node *)malloc(sizeof(struct node));
    head->next = NULL;

    // Do something
    for (int i = 0; i < 10; i++){
        next_node = head->next;
        head->next = next_node->next;
        free(next_node);
    }
    free(head);
    return 0;
}
