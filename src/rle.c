#include "rle.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

char *rle_encode(const char *in_buf, size_t in_len, size_t *out_len)
{
    // whats been written to the buffer
    size_t j = 0;
    // max size needed for run counts and such
    char *enc = malloc(2 * in_len + 32);
    if (!enc)
        return NULL;

    for (size_t i = 0; i < in_len; ++i)
    // for every charavter in the input
    {
        // safety so that we can distinguish special characters later
        unsigned char c = (unsigned char)in_buf[i];
        if (c == '\\' || isdigit(c))
        {
            enc[j++] = '\\';
            enc[j++] = (char)c;
        }
        else
        {
            enc[j++] = (char)c;
        }

        size_t run = 1;
        // counting the number of idenitcal character in a run
        while (i + 1 < in_len && in_buf[i + 1] == (char)c)
        {
            ++run;
            ++i;
        }
        // formatting the run here
        if (run > 1)
        {
            char num[32];
            int n = snprintf(num, sizeof num, "%zu", run);
            memcpy(enc + j, num, (size_t)n);
            j += (size_t)n;
        }
    }
    // null sentinel for my boi bwt later
    enc[j] = '\0';
    // final length
    *out_len = j;
    return enc;
}

char *rle_decode(const char *in_buf, size_t in_len, size_t *out_len)
{
    // start with double‐size buffer
    size_t cap = (in_len + 1) * 2;
    char *dec = malloc(cap);
    if (!dec) {
        return NULL;
    }
        

    size_t i = 0;
    size_t j = 0;
    while (i < in_len) // eat the whole input
    {
        // read character
        char c = in_buf[i++];
        // when we have a backslash we know we're dealing with a silly character
        if (c == '\\' && i < in_len)
        {
            c = in_buf[i++];
        }
        // parse run length 
        size_t run = 0;
        while (i < in_len && isdigit((unsigned char)in_buf[i]))
        {
            run = run * 10 + (in_buf[i++] - '0');
        }
        if (run == 0) {
            run = 1;
        }
        
        // grow if needed
        if (j + run >= cap)
        {
            cap *= 2;
            char *tmp = realloc(dec, cap);
            if (!tmp)
            {
                free(dec);
                return NULL;
            }
            dec = tmp;
        }

        // write out the run
        for (size_t k = 0; k < run; ++k)
        {
            dec[j++] = c;
        }
    }

    dec[j] = '\0';
    *out_len = j;
    return dec;
}
