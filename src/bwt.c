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

static void counting_sort(int *sa, int *rank, int n, int k, int *tmp)
{
    int maxv = (n > 256) ? n : 256;          
    int *cnt = (int *)calloc(maxv + 1, sizeof(int));

    for (int i = 0; i < n; ++i) {
        int r = (i + k < n) ? rank[i + k] : 0;
        ++cnt[r];
    }
    for (int i = 1; i <= maxv; ++i)           
        cnt[i] += cnt[i - 1];

    for (int i = n - 1; i >= 0; --i) {        
        int idx = sa[i];
        int r   = (idx + k < n) ? rank[idx + k] : 0;
        tmp[--cnt[r]] = idx;
    }
    memcpy(sa, tmp, n * sizeof(int));
    free(cnt);
}


size_t bwt_encode2(const uint8_t *T, size_t n, uint8_t *L)
{
    size_t N = n + 1;                        
    uint8_t *S = (uint8_t *)malloc(N);
    memcpy(S, T, n);
    S[n] = 0;                                 
    int *sa   = (int *)malloc(N * sizeof(int));
    int *rank = (int *)malloc(N * sizeof(int));
    int *tmp  = (int *)malloc(N * sizeof(int));

    for (size_t i = 0; i < N; ++i) {
        sa[i]   = (int)i;
        rank[i] = S[i];                      
    }

    for (int k = 1; k < (int)N; k <<= 1) {
        counting_sort(sa, rank, (int)N, k, tmp);
        counting_sort(sa, rank, (int)N, 0, tmp);

        tmp[sa[0]] = 1;
        for (size_t i = 1; i < N; ++i) {
            int cur = sa[i], prev = sa[i - 1];
            int r1  = rank[cur],   r2  = rank[prev];
            int s1  = (cur + k < (int)N) ? rank[cur + k]  : 0;
            int s2  = (prev + k < (int)N) ? rank[prev + k] : 0;
            tmp[cur] = tmp[prev] + (r1 != r2 || s1 != s2);
        }
        memcpy(rank, tmp, N * sizeof(int));
        if (rank[sa[N - 1]] == (int)N) break;   
    }

    size_t primary = 0;
    for (size_t i = 0; i < N; ++i) {
        int j = sa[i];
        L[i]  = (uint8_t)(j ? S[j - 1] : S[N - 1]);  
        if (j == 0) primary = i;
    }

    free(S); free(sa); free(rank); free(tmp);
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
