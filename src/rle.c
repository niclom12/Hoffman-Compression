#include "rle.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

//c char *enc = NULL; size_t cap = 4096, j = 0; enc = malloc(cap); … ensure(cap, j + 16); /* macro that doubles `cap` and reallocs */
#define ENSURE(buf, cap, need)                          \
    do {                                                \
        if ((need) > (cap)) {                           \
            size_t new_cap = (cap);                     \
            while (new_cap <= (need)) new_cap <<= 1;    \
            char *tmp__ = realloc((buf), new_cap);      \
            if (!tmp__) {                               \
                free(buf);                              \
                return NULL;                            \
            }                                           \
            (buf) = tmp__;                              \
            (cap) = new_cap;                            \
        }                                               \
    } while (0)

char *rle_encode(const char *in_buf, size_t in_len, size_t *out_len)
{
    size_t cap = 4096;
    size_t j   = 0;
    char  *enc = malloc(cap);
    if (!enc) return NULL;

    for (size_t i = 0; i < in_len; ++i) {
        unsigned char c = (unsigned char)in_buf[i];

        if (c == '\\' || isdigit(c)) {
            ENSURE(enc, cap, j + 2);  
            enc[j++] = '\\';
            enc[j++] = (char)c;      
        } else {
            ENSURE(enc, cap, j + 1);
            enc[j++] = (char)c;
        }

        size_t run = 1;
        while (i + 1 < in_len && in_buf[i + 1] == (char)c) {
            ++run;
            ++i;
        }
        if (run > 1) {
            char num[32];
            int  n = snprintf(num, sizeof num, "%zu", run);
            ENSURE(enc, cap, j + (size_t)n);
            memcpy(enc + j, num, (size_t)n);
            j += (size_t)n;
        }
    }

    ENSURE(enc, cap, j + 1);
    enc[j]   = '\0';      /* safety NUL – harmless */
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
