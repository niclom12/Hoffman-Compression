#include "rle.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

char *rle_encode(const char *in_buf, size_t in_len, size_t *out_len) {
    size_t max_size = in_len * 2 + 1;
    char *enc = malloc(max_size);
    if (!enc) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < in_len; ++i) {
        char c = in_buf[i];

        // escape digits and backslashes
        if (isdigit((unsigned char)c)) {
            enc[j++] = '\\';
        } else if (c == '\\') {
            enc[j++] = '\\';
            enc[j++] = '\\';
        }

        // write the character itself
        enc[j++] = c;

        // count run length
        size_t run = 1;
        while (i + 1 < in_len && in_buf[i + 1] == c) {
            run++;
            i++;
        }
        if (run > 1) {
            // append the count
            int written = snprintf(enc + j, max_size - j, "%zu", run);
            if (written < 0 || (size_t)written >= max_size - j) {
                // should never happen unless out of space
                free(enc);
                return NULL;
            }
            j += (size_t)written;
        }
    }

    enc[j] = '\0';
    *out_len = j;
    return enc;
}

char *rle_decode(const char *in_buf, size_t in_len, size_t *out_len) {
    // start with double‐size buffer
    size_t cap = (in_len + 1) * 2;
    char *dec = malloc(cap);
    if (!dec) return NULL;

    size_t i = 0, j = 0;
    while (i < in_len) {
        // read character (handling escape)
        char c = in_buf[i++];
        if (c == '\\' && i < in_len) {
            c = in_buf[i++];
        }
        // parse run length (if any)
        size_t run = 0;
        while (i < in_len && isdigit((unsigned char)in_buf[i])) {
            run = run * 10 + (in_buf[i++] - '0');
        }
        if (run == 0) run = 1;

        // grow if needed
        if (j + run >= cap) {
            cap *= 2;
            char *tmp = realloc(dec, cap);
            if (!tmp) {
                free(dec);
                return NULL;
            }
            dec = tmp;
        }

        // write out the run
        for (size_t k = 0; k < run; ++k) {
            dec[j++] = c;
        }
    }

    dec[j] = '\0';
    *out_len = j;
    return dec;
}
