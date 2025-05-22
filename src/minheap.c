/**
 * This is the file that contains the minheap logic. We will use this data structure to create a priority queue and assign codes that way.
 */
#include <stdio.h>
#include <stdlib.h>
#include "minheap.h"

void minheap_init(MinHeap *heap) {
    heap->size = 0;
}

void swap(Node **a, Node **b) {
    Node *temp = *a;
    *a = *b;
    *b = temp;
}

void heapify_up(MinHeap *heap, int index) {
    if (index == 0) return;
    int parent = (index - 1) / 2;
    if (heap->data[index]->freq < heap->data[parent]->freq) {
        swap(&heap->data[index], &heap->data[parent]);
        heapify_up(heap, parent);
    }
}

void heapify_down(MinHeap *heap, int index) {
    int left = 2 * index + 1;
    int right = 2 * index + 2;
    int smallest = index;

    if (left < heap->size && heap->data[left]->freq < heap->data[smallest]->freq)
        smallest = left;
    if (right < heap->size && heap->data[right]->freq < heap->data[smallest]->freq)
        smallest = right;

    if (smallest != index) {
        swap(&heap->data[index], &heap->data[smallest]);
        heapify_down(heap, smallest);
    }
}

void minheap_insert(MinHeap *heap, Node *node) {
    if (heap->size >= MAX_HEAP_SIZE) {
        fprintf(stderr, "Heap overflow\n");
        exit(1);
    }
    heap->data[heap->size] = node;
    heapify_up(heap, heap->size);
    heap->size++;
}

Node* minheap_extract_min(MinHeap *heap) {
    if (heap->size == 0) return NULL;
    Node* min = heap->data[0];
    heap->size--;
    heap->data[0] = heap->data[heap->size];
    heapify_down(heap, 0);
    return min;
}

Node* create_node(__uint8_t ch, int freq) {
    Node* node = malloc(sizeof(Node));
    node->ch = ch;
    node->freq = freq;
    node->left = node->right = NULL;
    return node;
}

int unit_test() {
    MinHeap heap;
    minheap_init(&heap);

    minheap_insert(&heap, create_node('A', 5));
    minheap_insert(&heap, create_node('B', 2));
    minheap_insert(&heap, create_node('C', 7));

    Node *min = minheap_extract_min(&heap);
    printf("Min char: %c, freq: %d\n", min->ch, min->freq);

    return 0;
}