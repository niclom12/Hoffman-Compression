#include "bwt.h"
#include <stdlib.h>
#include <string.h>
#include "huffsize.h"
#include "suffix_array.h"

// struct to hold just the rotation start index. 
typedef struct { 
    size_t index_of_sentinel; 
} rotation_t;


size_t bwt_encode(const uint8_t *input, size_t len, uint8_t *bwt_out)
{
    // length plut the sentinel character
    size_t n = len + 1;                
    uint8_t *buffer = malloc(n);
    if (!buffer) {
        return (size_t)-1;
    }
    memcpy(buffer, input, len); // copy input to buffer
    buffer[len] = 0; // unique min      

    // now we allocate the space for the suffix array
    int *suffix_array = malloc(n * sizeof *suffix_array);
    if (!suffix_array) { 
        free(buffer); 
        return (size_t)-1; 
    }

    // if we get anything but 0, so when we return -1
    if (build_suffix_array(buffer, n, suffix_array)) {
        free(suffix_array); 
        free(buffer); 
        return (size_t)-1;
    }


    size_t primary = 0;
    for (size_t i = 0; i < n; ++i) {
        size_t j = (suffix_array[i] + n - 1) % n;   
        bwt_out[i] = buffer[j]; // get corresponfing sysmbol 
        if (suffix_array[i] == 0) {
            primary = i;
        }    
    }

    free(suffix_array);
    free(buffer);
    return primary;
}

uint8_t *bwt_decode(const uint8_t *bwt, size_t len, size_t primary) {
    
    size_t n = len;
    uint8_t *first = malloc(n);
    if (!first) {
        return NULL;
    }
    
    memcpy(first, bwt, n);
    size_t count[128] = {0};
    for (size_t i = 0; i < n; i++) {
        count[first[i]]++;
    }
    size_t pos[128];
    size_t sum = 0;
    for (int c = 0; c < 128; c++) {
        pos[c] = sum;
        sum   += count[c];
    }


    size_t *LF = malloc(n * sizeof *LF);
    if (!LF) { 
        free(first); 
        return NULL; 
    }

    memset(count, 0, sizeof count);
    for (size_t i = 0; i < n; i++) {
        uint8_t c = bwt[i];
        LF[i] = pos[c] + (count[c]++);
    }

    uint8_t *tmp = malloc(n);
    if (!tmp) { 
        free(LF);
        free(first); 
        return NULL; 
    }
    
    size_t idx = primary;
    for (size_t i = n; i-- > 0; ) {
        tmp[i] = bwt[idx];
        idx = LF[idx];
    }

    free(LF);
    free(first);
    return tmp;  
}
