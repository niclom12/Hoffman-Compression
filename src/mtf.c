#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "mtf.h"

unsigned char *mtf_encode(const unsigned char *in,
                                 size_t n, size_t *out_len)
{
    unsigned char *out = malloc(n);
    if (!out) return NULL;

    /* initial list 0..255 */
    unsigned char list[256];
    for (int i = 0; i < 256; ++i) list[i] = (unsigned char)i;

    *out_len = 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = in[i];

        /* find c’s current position */
        int j = 0;
        while (list[j] != c) ++j;       /* 0-255 worst-case */

        out[i] = (unsigned char)j;      /* write index     */
        *out_len = i+1;

        /* move to front */
        while (j) { list[j] = list[j-1]; --j; }
        list[0] = c;
    }
    return out;
}

unsigned char *mtf_decode(const unsigned char *indices,
                                 size_t n, size_t *out_len)
{
    unsigned char *out = malloc(n);
    if (!out) return NULL;

    unsigned char list[256];
    for (int i = 0; i < 256; ++i) list[i] = (unsigned char)i;
    *out_len = 0;
    for (size_t i = 0; i < n; ++i) {
        int k = indices[i];                 /* 0-255 */
        unsigned char c = list[k];
        out[i] = c;
        *out_len = i+1;

        /* move c to front */
        while (k) { list[k] = list[k-1]; --k; }
        list[0] = c;
    }
    return out;
}
