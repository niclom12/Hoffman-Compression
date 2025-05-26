#ifndef MINHEAP_H
#define MINHEAP_H

#define MAX_HEAP_SIZE 256

typedef struct Node {
    char ch;
    int freq;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct {
    int size;
    Node* data[MAX_HEAP_SIZE];
} MinHeap;

void minheap_init(MinHeap *heap);
void minheap_insert(MinHeap *heap, Node *node);
Node* minheap_extract_min(MinHeap *heap);
int unit_test();

#endif
