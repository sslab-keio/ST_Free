#include <stdio.h>
#include <stdlib.h>

struct node_device {
  void* data;
};

void node_acquire(struct node_device*);
void node_destroy(struct node_device*);


struct node_ops {
  void (*resource_acquire)(struct node_device*);
  void (*resource_destroy)(struct node_device*);
};

struct node {
  void* data;
  int x;
  struct node_ops ops;
};

struct node_ops nops = {
  .resource_acquire = node_acquire,
  .resource_destroy = node_destroy,
};

struct node gnode;

void node_acquire(struct node_device* n) {
  return;
}

void node_destroy(struct node_device* n) {
  return;
}

int main(){
  struct node *nd;
  struct node_device *ndv;
  nd = (struct node*)malloc(sizeof(struct node));
  nd->ops = nops;

  nd->ops.resource_acquire(ndv);

  nd->ops.resource_destroy(ndv);
  return 0;
}
