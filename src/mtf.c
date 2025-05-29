#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "mtf.h"

uint8_t *mtf_encode(const uint8_t *in, size_t n, size_t *out_len) {
    uint8_t *out = malloc(n);
    if (!out) return NULL;

    // initial list 0..255
    uint8_t list[256];
    for (int i = 0; i < 256; i++) list[i] = (uint8_t)i;

    for (size_t i = 0; i < n; i++) {
        uint8_t c = in[i];
        // find c
        int j = 0;
        while (list[j] != c) j++;
        // write index
        out[i] = (uint8_t)j;
        *out_len = i+1;
        // move to front: shift list[0..j-1] right, place c at list[0]
        memmove(&list[1], &list[0], j);
        list[0] = c;
    }
    return out;
}

uint8_t *mtf_decode(const uint8_t *indices, size_t n, size_t *out_len) {
    uint8_t *out = malloc(n);
    if (!out) return NULL;

    // initial list 0..255
    uint8_t list[256];
    for (int i = 0; i < 256; i++) list[i] = (uint8_t)i;

    for (size_t i = 0; i < n; i++) {
        int k = indices[i];        // 0..255
        uint8_t c = list[k];
        out[i] = c;
        *out_len = i+1;

        // move c to front
        memmove(&list[1], &list[0], k);
        list[0] = c;
    }
    return out;
}