#include "bwt.h"
#include <stdlib.h>
#include <string.h>
#include "huffsize.h"
#include "suffix_array.h"

// struct to hold just the rotation start index. 
typedef struct { 
    size_t index_of_sentinel; 
} rotation_t;

// counting sort nlogn approach
size_t bwt_encode_count_sort(const uint8_t *T, size_t n, uint8_t *L)
{
    //size with the addition of the sentinel
    size_t N = n + 1; 
    uint8_t *S = (uint8_t *)malloc(N);
    memcpy(S, T, n);
    // adding the sentinel
    S[n] = 0;                

    // suffix array 
    int *sa   = (int *)malloc(N * sizeof(int));
    // ranks for counting 
    int *rank = (int *)malloc(N * sizeof(int));
    // temp array for transfering 
    int *tmp  = (int *)malloc(N * sizeof(int));

    // intializing the values of the suffix array and rank
    for (size_t i = 0; i < N; ++i) {
        sa[i]   = (int)i; // identity perm of the character
        rank[i] = S[i]; // value of the character at that index      
    }
    // counting sort approach with prefix doubling
    // k is the length of the prefix whose second half we are sorting on.
    for (int k = 1; k < (int)N; k <<= 1) { // we're shifting here so it aint just N
        // radix-sort suffixes by (rank[i+k], rank[i]) using two 
        // counting sorts:  LS-key first, then MS-key.
        counting_sort(sa, rank, (int)N, k, tmp);
        counting_sort(sa, rank, (int)N, 0, tmp);

        // rebuild ranks based on the newly sorted order
        tmp[sa[0]] = 1; // ranks start from 1 bc 0 is for the sentiel
        for (size_t i = 1; i < N; ++i) {
            int cur = sa[i];  // current suffix start
            int prev = sa[i - 1]; // previous prefix

            // compare (rank[cur], rank[cur+k]) to previous
            int r1  = rank[cur];
            int r2  = rank[prev];
            int s1  = (cur + k < (int)N) ? rank[cur + k]  : 0;
            int s2  = (prev + k < (int)N) ? rank[prev + k] : 0;
            // add 1 when pair differs, else copy previous rank
            tmp[cur] = tmp[prev] + (r1 != r2 || s1 != s2);
        }
        // exit if the largest rank is N, every suffix already owns a unique rank
        memcpy(rank, tmp, N * sizeof(int));
        if (rank[sa[N - 1]] == (int)N) break;   
    }

    // now that we have the sorted suffix array we can produce the BWT 
    // which is the last column
    size_t primary = 0;
    for (size_t i = 0; i < N; ++i) {
        int j = sa[i];
        L[i]  = (uint8_t)(j ? S[j - 1] : S[N - 1]);  
        if (j == 0) {
             primary = i;
        }
    }

    free(S); 
    free(sa); 
    free(rank); 
    free(tmp);
    return primary;
}
// more of a radix sort 
void counting_sort(int *sa, int *rank, int n, int k, int *tmp)
{
    // 0 to 255 after first round, or 1 to n for prefix-doubling
    int maxv = (n > 256) ? n : 256;          
    int *cnt = (int *)calloc(maxv + 1, sizeof(int));

    // this is the histogram pass, where we count up the occurances, could be optimized by using huffman's frequ
    for (int i = 0; i < n; ++i) {
        int r = (i + k < n) ? rank[i + k] : 0; // rank of 2nd half or 0 for sentinel
        ++cnt[r];
    }
    // this is prefix-sum pass of cumulative pass
    for (int i = 1; i <= maxv; ++i)           
        cnt[i] += cnt[i - 1];
    
    // placement
    for (int i = n - 1; i >= 0; --i) {        
        int idx = sa[i]; // suffix start position
        int r = (idx + k < n) ? rank[idx + k] : 0;
        tmp[--cnt[r]] = idx; // --cnt gives last free slot
    }
    memcpy(sa, tmp, n * sizeof(int));
    free(cnt);
}

uint8_t *bwt_decode(const uint8_t *bwt, size_t len, size_t primary) {
    size_t n = len;
    uint8_t *first = malloc(n);
    if (!first) {
        return NULL;
    }
    memcpy(first, bwt, n);
    // counting frequencies of the characters
    size_t count[128] = {0};
    for (size_t i = 0; i < n; i++) {
        count[first[i]]++;
    }
    // prefix sums
    size_t pos[128]; // first 'row' where that character appears
    size_t sum = 0;
    for (int c = 0; c < 128; c++) {
        pos[c] = sum; 
        sum += count[c];
    }
    // After the loop, the rows constitutes the block of chars in the first col

    // building the LF mapping - last - first mapping
    size_t *LF = malloc(n * sizeof *LF);
    if (!LF) { 
        free(first); 
        return NULL; 
    }

    memset(count, 0, sizeof count); // histogram as counters
    for (size_t i = 0; i < n; i++) {
        // row i corresponds to the (count[c])-th occurrence of that character in the last column. 
        // In the first column that same occurrence lives at row  pos[c] + count[c].
        uint8_t c = bwt[i];
        LF[i] = pos[c] + (count[c]++);
    }
    // now LF is a permutation of 0 to n-1  
    // reconstruct the original text
    uint8_t *tmp = malloc(n);
    if (!tmp) { 
        free(LF);
        free(first); 
        return NULL; 
    }

    // Walk LF backwards starting at primary 
    // Each step prepends one character to the output 
    size_t idx = primary;
    for (size_t i = n; i-- > 0; ) {
        tmp[i] = bwt[idx];
        idx = LF[idx];
    }

    free(LF);
    free(first);
    return tmp;  
}

// naive approach O(n^2logn)
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
    if ((suffix_array)) {
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