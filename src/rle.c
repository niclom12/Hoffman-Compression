#include "rle.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

uint8_t *rle_encode(const uint8_t *in_buf, size_t in_len, size_t *out_len) {
    // worst case every byte is escaped plus a count, so 2×in_len+1 is safe
    size_t max_size = in_len * 2 + 1;
    uint8_t *enc = malloc(max_size);
    if (!enc) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < in_len; ++i) {
        uint8_t c = in_buf[i];

        // escape digits and backslashes
        if (isdigit(c)) {
            enc[j++] = '\\';
        } else if (c == '\\') {
            enc[j++] = '\\';
            enc[j++] = '\\';
        }

        // write the byte itself
        enc[j++] = c;

        // count run length
        size_t run = 1;
        while (i+1 < in_len && in_buf[i+1] == c) {
            run++; i++;
        }
        if (run > 1) {
            // append decimal count, as text
            char numbuf[32];
            int written = snprintf(numbuf, sizeof numbuf, "%zu", run);
            if (written < 0 || (size_t)written > sizeof numbuf) {
                free(enc);
                return NULL;
            }
            // copy those ASCII digits into enc[]
            for (int k = 0; k < written; k++) {
                enc[j++] = (uint8_t)numbuf[k];
            }
        }
    }

    *out_len = j;
    return enc;
}

uint8_t *rle_decode(const uint8_t *in_buf, size_t in_len, size_t *out_len) {
    // start with a buffer twice as large as input
    size_t cap = (in_len + 1) * 2;
    uint8_t *dec = malloc(cap);
    if (!dec) return NULL;

    size_t i = 0, j = 0;
    while (i < in_len) {
        // read byte (handling escape)
        uint8_t c = in_buf[i++];
        if (c == '\\' && i < in_len) {
            c = in_buf[i++];
        }

        // parse decimal run length (if any)
        size_t run = 0;
        while (i < in_len && isdigit(in_buf[i])) {
            run = run * 10 + (in_buf[i++] - '0');
        }
        if (run == 0) run = 1;

        // ensure capacity
        if (j + run > cap) {
            do { cap *= 2; } while (j + run > cap);
            uint8_t *tmp = realloc(dec, cap);
            if (!tmp) { free(dec); return NULL; }
            dec = tmp;
        }
        // output the run
        for (size_t k = 0; k < run; k++) {
            dec[j++] = c;
        }
    }

    *out_len = j;
    return dec;
}