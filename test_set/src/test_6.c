#include <stdio.h>
#include <stdlib.h>

struct node{
    int x;
    struct node * next;
};

int main(){
    struct node * head, * next_node;
    head = (struct node *)malloc(sizeof(struct node));
    head->next = NULL;

    for (int i = 0; i < 10; i++){
        next_node = (struct node *)malloc(sizeof(struct node));
        next_node->next = head->next;
        head->next = next_node;
    }
    free(head);
    return 0;
}
