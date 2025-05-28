#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

/* opaque – we only need the freq field for comparisons */
struct node {
    int      freq;
    int      left;
    int      right;
    uint8_t  sym;
};

// swim the node up
void swim(int *heap, size_t idx, const struct node nodes[]);
// sink the node down
void sink(int *heap, size_t size, size_t idx, const struct node nodes[]);

#endif
