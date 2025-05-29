#include "suffix_array.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// this code was heavily inspired by the suffix array version of BWT from geeks for geeks 
// https://www.geeksforgeeks.org/suffix-array-set-2-a-nlognlogn-algorithm/
struct suffix {
    int index; // index in the string
    int rank[2]; // rank of the current character and rank of the adjacent character
};

// comparator for quicksorting
static int cmp_suffix(const void *a, const void *b)
{
    const struct suffix *sa = a;
    const struct suffix *sb = b;
    if (sa->rank[0] != sb->rank[0]) { 
        return sa->rank[0] - sb->rank[0]; 
    }
    return sa->rank[1] - sb->rank[1];
}

int build_suffix_array(const uint8_t *txt, size_t n, int *suffixArr)
{
    struct suffix *suffixes = malloc(n * sizeof *suffixes);
    int *ind = malloc(n * sizeof *ind);
    if (!suffixes || !ind) { 
        free(suffixes); 
        free(ind); 
        return -1; 
    }

    for (size_t i = 0; i < n; ++i) {
        suffixes[i].index   = (int)i;
        suffixes[i].rank[0] = (unsigned)txt[i];                
        suffixes[i].rank[1] = (i + 1 < n) ? (unsigned)txt[i + 1] : -1; // make sure that we can assign an adjactent character
    }
    qsort(suffixes, n, sizeof *suffixes, cmp_suffix); // sort the first char
   
    for (int k = 4; k < (int)2 * n; k <<= 1) {
        int rank = 0;
        int prev0 = suffixes[0].rank[0];
        int prev1 = suffixes[0].rank[1];

        suffixes[0].rank[0] = rank;
        ind[suffixes[0].index] = 0;

        for (size_t i = 1; i < n; ++i) {
            if (suffixes[i].rank[0] == prev0 && suffixes[i].rank[1] == prev1) {
                suffixes[i].rank[0] = rank;
            } else {
                prev0 = suffixes[i].rank[0];
                prev1 = suffixes[i].rank[1];
                suffixes[i].rank[0] = ++rank;
            }
            ind[suffixes[i].index] = (int)i;
        }

        // set next rank
        for (size_t i = 0; i < n; ++i) {
            int next = suffixes[i].index + k / 2;
            suffixes[i].rank[1] = (next < (int)n) ? suffixes[ind[next]].rank[0] : -1;
        }
        qsort(suffixes, n, sizeof *suffixes, cmp_suffix); //sort at the next index
    }

    // storing the suffix array
    for (size_t i = 0; i < n; ++i) {
        suffixArr[i] = suffixes[i].index;
    }
        
    free(suffixes);
    free(ind);
    return 0;
}
