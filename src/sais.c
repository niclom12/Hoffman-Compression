/*
 * suffix_array_sais.c
 *
 * Lightweight SA-IS implementation and integration for BWT.
 * Replaces prefix-doubling suffix array in BWT.
 */

#include "suffix_array.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// SA-IS induced sorting
static void get_buckets(const uint8_t *T, int *C, int *B, int n, int K, int end) {
    memset(C, 0, K * sizeof(int));
    for (int i = 0; i < n; ++i) C[T[i]]++;
    int sum = 0;
    for (int i = 0; i < K; ++i) {
        sum += C[i];
        B[i] = end ? sum-1 : (sum - C[i]);
    }
}

static void induce_L(const uint8_t *T, int *SA, uint8_t *type, int *C, int *B, int n, int K) {
    get_buckets(T, C, B, n, K, 0);
    for(int i=0;i<n;++i) {
        int j = SA[i] - 1;
        if (j>=0 && !type[j]) {
            SA[B[T[j]]++] = j;
        }
    }
}

static void induce_S(const uint8_t *T, int *SA, uint8_t *type, int *C, int *B, int n, int K) {
    get_buckets(T, C, B, n, K, 1);
    for(int i=n-1;i>=0;--i) {
        int j = SA[i] - 1;
        if (j>=0 && type[j]) {
            SA[B[T[j]]--] = j;
        }
    }
}

int sais(const uint8_t *T, int *SA, int n, int K) {
    if (n==0) return 0;
    // 1. classify
    uint8_t *type = malloc(n);
    type[n-1] = 1; // S-type
    for (int i = n-2; i >= 0; --i) {
        if (T[i] < T[i+1] || (T[i]==T[i+1] && type[i+1])) type[i]=1;
        else type[i]=0;
    }
    // 2. bucket sizes
    int *C = malloc(K*sizeof(int));
    int *B = malloc(K*sizeof(int));
    // 3. place LMS
    memset(SA, -1, n*sizeof(int));
    get_buckets(T,C,B,n,K,1);
    for(int i=1;i<n;++i) if(type[i] && !type[i-1]) SA[B[T[i]]--]=i;
    // 4. induce L and S
    induce_L(T,SA,type,C,B,n,K);
    induce_S(T,SA,type,C,B,n,K);

    // 5. extract LMS substrings
    int m=0;
    for(int i=1;i<n;i++) if(type[i] && !type[i-1]) m++;
    int *lms = malloc(m*sizeof(int));
    for(int i=1, j=0;i<n;i++) if(type[i] && !type[i-1]) lms[j++]=i;

    int *name = malloc(m*sizeof(int));
    int prev = -1, np = 0;
    for(int i=0;i<n;i++) {
        int p = SA[i];
        if (p>0 && type[p] && !type[p-1]) {
            int idx = -1;
            for(int k=0; k<m; k++) if(lms[k]==p) { idx=k; break; }
            if (prev<0) {
                name[idx]=np;
            } else {
                // compare substrings
                int diff=0;
                for(int a=prev, b=p; ; a++,b++) {
                    if (T[a]!=T[b] || type[a]!=type[b]) { diff=1; break; }
                    if ((a>0 && type[a] && !type[a-1]) || (b>0 && type[b] && !type[b-1])) break;
                }
                if (diff) ++np;
                name[idx]=np;
            }
            prev=p;
        }
    }
    np++;

    // 6. recurse or direct
    int *SA1 = malloc(m*sizeof(int));
    int *T1  = malloc(m*sizeof(int));
    for(int i=0;i<m;i++) T1[i]=name[i];
    if (np < m) {
        sais((uint8_t*)T1, SA1, m, np);
    } else {
        for(int i=0;i<m;i++) SA1[T1[i]] = i;
    }

    // 7. induce final SA
    memset(SA, -1, n*sizeof(int));
    get_buckets(T,C,B,n,K,1);
    for(int i=m-1;i>=0;i--) {
        int p = lms[SA1[i]];
        SA[B[T[p]]--] = p;
    }
    induce_L(T,SA,type,C,B,n,K);
    induce_S(T,SA,type,C,B,n,K);

    free(type); free(C); free(B);
    free(lms); free(name); free(SA1); free(T1);
    return 0;
}

// wrapper
int build_suffix_array_sais(const uint8_t *T, size_t n, int *SA) {
    return sais(T, SA, (int)n, 256);
}