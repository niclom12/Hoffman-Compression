#include "heap.h"
#include "huffman.h"  
#include <stddef.h>

// swaps the indexes of notes
static inline void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}


void swim(int *heap, size_t idx, const struct node nodes[])
{
    while (idx && nodes[heap[idx]].freq < nodes[heap[(idx-1)>>1]].freq) {
        size_t parent = (idx-1) >> 1;
        swap(&heap[idx], &heap[parent]);
        idx = parent;
    }
}

void sink(int *heap, size_t size, size_t idx, const struct node nodes[])
{
    for (;;) {
        size_t l = (idx<<1) + 1;
        size_t r = l + 1;
        size_t smallest = idx;

        if (l < size && nodes[heap[l]].freq < nodes[heap[smallest]].freq)
            smallest = l;
        if (r < size && nodes[heap[r]].freq < nodes[heap[smallest]].freq)
            smallest = r;

        if (smallest == idx) break;
        swap(&heap[idx], &heap[smallest]);
        idx = smallest;
    }
}
