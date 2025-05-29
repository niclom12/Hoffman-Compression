// dc3_bwt_file.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dc3.h"

#define ALPHABET 128
typedef struct
{
    int idx, first, second;
} pair_t;
// ----------------------------------------------------------------------
// fileio.c snippet: read entire file into a NUL-terminated buffer
// ----------------------------------------------------------------------

int cmp_pair(const void *a, const void *b)
{
    const pair_t *p1 = a, *p2 = b;
    if (p1->first != p2->first)
        return p1->first - p2->first;
    return p1->second - p2->second;
}

// char *read_text_file_to_buffer(const char *filename, long *length)
// {
//     FILE *fp = fopen(filename, "rb");
//     if (!fp)
//         return NULL;
//     fseek(fp, 0, SEEK_END);
//     *length = ftell(fp);
//     rewind(fp);

//     char *buffer = malloc((size_t)*length + 1);
//     if (!buffer)
//     {
//         fclose(fp);
//         return NULL;
//     }

//     size_t ret = fread(buffer, 1, (size_t)*length, fp);
//     fclose(fp);
//     if (ret < (size_t)*length)
//     {
//         free(buffer);
//         return NULL;
//     }
//     buffer[*length] = '\0';
//     return buffer;
// }

// ----------------------------------------------------------------------
// Prototypes for our DC3 + BWT routines (same as before)
// ----------------------------------------------------------------------
void build_dc3_sa(int *T, int n, int *SA);

// // ----------------------------------------------------------------------
// // Main: load text file, build SA, print SA and BWT
// // ----------------------------------------------------------------------
// int main(int argc, char **argv)
// {

//     // const char *bwt = "ard$rcaaaabb";
//     // size_t n = strlen(bwt);

//     // printf("BWT input: %s\n", bwt);

//     // char *decoded = invert_bwt(bwt, n);
//     // printf("Decoded : %s\n", decoded);

//     // free(decoded);

//     if (argc < 2)
//     {
//         fprintf(stderr, "Usage: %s <input.txt>\n", argv[0]);
//         return 1;
//     }

//     long length;
//     char *input = read_text_file_to_buffer(argv[1], &length);
//     if (!input)
//     {
//         fprintf(stderr, "Error reading '%s'\n", argv[1]);
//         return 1;
//     }

//     // Allocate T and SA dynamically based on file length + 1 sentinel
//     int n = (int)length + 1;
//     int *T = malloc(sizeof(int) * n);
//     int *SA = malloc(sizeof(int) * n);
//     char *BWT = malloc(n);

//     // Copy file bytes into T[0..n-2], sentinel at T[n-1]=0
//     for (int i = 0; i < n - 1; i++)
//     {
//         unsigned char c = input[i];
//         T[i] = (c < ALPHABET ? c : 0);
//     }
//     T[n - 1] = 0;
//     free(input);

//     // Build suffix array via DC3 (stubbed parts)
//     build_dc3_sa(T, n, SA);

//     // Print the SA
//     // printf("\n=== Final Suffix Array ===\n");
//     // for (int i = 0; i < n; i++)
//     // {
//     //     printf("SA[%2d] = %2d\n", i, SA[i]);
//     // }

//     // Compute and print the BWT
//     compute_bwt(T, n, SA, BWT);
//     printf("\n=== Burrows–Wheeler Transform ===\n");
//     for (int i = 0; i < n; i++)
//     {
//         if (BWT[i] == 0)
//             putchar('$');
//         else
//             putchar(BWT[i]);
//     }
//     putchar('\n');

//     // Cleanup
//     free(T);
//     free(SA);
//     free(BWT);
//     return 0;
// }

// ----------------------------------------------------------------------
// build_dc3_sa: exactly the same stubbed skeleton as before
// ----------------------------------------------------------------------
void build_dc3_sa(int *T, int n, int *SA)
{
    printf("build dc3 \n");
    // 1) Identify sample positions S1 = { i | i mod 3 != 0 }
    size_t S1sz = 0;
    size_t *S1 = malloc(n * sizeof *S1);
    for (size_t i = 0; i < n; i++)
        if (i % 3 != 0)
            S1[S1sz++] = i;

    // printf("Sample positions (mod 1/2):");
    // for (int i = 0; i < S1sz; i++)
    //     printf(" %d", S1[i]);
    // printf("\n");

    // 2) Form and print triples for each sample
    for (int i = 0; i < S1sz; i++)
    {
        int idx = S1[i];
        int a = T[idx],
            b = (idx + 1 < n ? T[idx + 1] : 0),
            c = (idx + 2 < n ? T[idx + 2] : 0);
        // printf("  Triple@%d: (%d,'%c'),(%d,'%c'),(%d,'%c')\n",
        //        idx, a, a ? a : '$', b, b ? b : '$', c, c ? c : '$');
    }

    // 3) Radix‐sort those triples into SA1[]
    int *SA1 = malloc(sizeof(int) * S1sz);
    int *tmp = malloc(sizeof(int) * S1sz);
    
    // init SA1 = S1
    for (size_t i = 0; i < S1sz; i++)
        SA1[i] = S1[i];

    for (int offset = 2; offset >= 0; offset--)
    {
        // --- determine the maximum key for *this* pass ---
        int sigma = 0;
        for (size_t j = 0; j < S1sz; j++)
        {
            int idx = SA1[j] + offset;
            int key = (idx < n ? T[idx] : 0);
            if (key > sigma)
                sigma = key;
        }
        sigma += 1; // we need counts[0..sigma-1]

        int *count = calloc(sigma, sizeof *count);

        // 3a) count keys
        for (size_t j = 0; j < S1sz; j++)
        {
            int idx = SA1[j] + offset;
            int key = (idx < n ? T[idx] : 0);
            count[key]++;
        }
        // 3b) prefix sums
        int sum = 0;
        for (int k = 0; k < sigma; k++)
        {
            int t = count[k];
            count[k] = sum;
            sum += t;
        }
        // 3c) reorder
        for (size_t j = 0; j < S1sz; j++)
        {
            int idx = SA1[j];
            int key = (idx + offset < n ? T[idx + offset] : 0);
            tmp[count[key]++] = idx;
        }
        // 3d) copy back
        for (size_t j = 0; j < S1sz; j++)
        {
            SA1[j] = tmp[j];
        }

        free(count);
    }

    // print the result of radix sort
    // printf("Sorted sample SA1:");
    // for (int i = 0; i < S1sz; i++)
    // {
    //     printf(" %d", SA1[i]);
    // }
    // printf("\n"); // should print: 5 1 4 2

    // 4) Assign ranks → R = [2,4,3,1]
    // Create a mapping from position → rank
    int *rank = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++)
        rank[i] = 0;

    // SA1 is in ascending order of the triples,
    rank[SA1[0]] = 1;

    // For each subsequent sample, compare its triple to the previous one.
    // If equal, same rank; otherwise increment.
    for (size_t i = 1; i < S1sz; i++)
    {
        int prev = SA1[i - 1], cur = SA1[i];

        int a0 = T[prev],
            b0 = (prev + 1 < n ? T[prev + 1] : 0),
            c0 = (prev + 2 < n ? T[prev + 2] : 0);

        int a1 = T[cur],
            b1 = (cur + 1 < n ? T[cur + 1] : 0),
            c1 = (cur + 2 < n ? T[cur + 2] : 0);

        if (a0 == a1 && b0 == b1 && c0 == c1)
        {
            // same triple → same rank
            rank[cur] = rank[prev];
        }
        else
        {
            // new triple → bump rank
            rank[cur] = rank[prev] + 1;
        }
    }

    // Now max_rank = number of distinct triples
    int max_rank = rank[SA1[S1sz - 1]];
    if (max_rank < (int)S1sz)
    {
        // Build R2 = [ rank[S1[0]], …, rank[S1[S1sz-1]], 0 ]
        int *R2 = malloc((S1sz + 1) * sizeof *R2);
        int *SA12 = malloc((S1sz + 1) * sizeof *SA12);
        for (size_t i = 0; i < S1sz; i++)
        {
            R2[i] = rank[S1[i]];
        }
        R2[S1sz] = 0; // sentinel for recursion

        // Recurse: sort R2 to get SA12
        printf("Recursing\n");
        build_dc3_sa(R2, (int)(S1sz + 1), SA12);
        printf("Recursed\n");

        // Remap SA12 back to SA1
        // SA12 contains all positions in R2 (0..S1sz),
        // but we only want those < S1sz, mapped via S1[].
        size_t k = 0;
        for (size_t i = 0; i < S1sz + 1; i++)
        {
            if (SA12[i] < (int)S1sz)
            {
                SA1[k++] = S1[SA12[i]];
            }
        }
        // Rebuild rank[] from fully-sorted SA1
        for (size_t i = 0; i < S1sz; i++)
        {
            rank[SA1[i]] = (int)i + 1;
        }

        free(R2);
        free(SA12);
    }

    // Build the reduced string R in the order of increasing S1[]
    int *R = malloc(sizeof(int) * S1sz);
    // printf("Reduced string R:");
    for (int i = 0; i < S1sz; i++)
    {
        R[i] = rank[S1[i]];
        // printf(" %d", R[i]);
    }
    // printf("\n");

    // 5a) collect S0 = { i | i mod 3 == 0 }
    size_t S0sz = 0;
    size_t *S0 = malloc(n * sizeof *S0);
    for (int i = 0; i < n; i++)
    {
        if (i % 3 == 0)
        {
            S0[S0sz++] = i;
        }
    }
    // printf("Non-sample S0 positions:");
    // for (int i = 0; i < S0sz; i++)
    //     printf(" %d", S0[i]);
    // printf("\n");

    pair_t *arr = malloc(sizeof(pair_t) * S0sz);
    for (int i = 0; i < S0sz; i++)
    {
        int idx = S0[i];
        arr[i].idx = idx;
        arr[i].first = T[idx];
        arr[i].second = (idx + 1 < n ? rank[idx + 1] : 0);
        // printf("  idx %d → (%d,'%c'), %d\n",
        //        idx,
        //        arr[i].first,
        //        arr[i].first ? arr[i].first : '$',
        //        arr[i].second);
    }

    qsort(arr, S0sz, sizeof(pair_t), cmp_pair);

    int *SA0 = malloc(sizeof(int) * S0sz);
    // printf("Sorted non-sample SA0:");
    for (int i = 0; i < S0sz; i++)
    {
        SA0[i] = arr[i].idx;
        // printf(" %d", SA0[i]);
    }
    // printf("\n\n");

    // free temporaries when you're done merging…
    // 6) Merge SA0 & SA1 into SA (stubbed merge for "banana")
    size_t p0 = 0, p1 = 0, p = 0;
    while (p0 < S0sz && p1 < S1sz)
    {
        size_t i0 = SA0[p0], i1 = SA1[p1];
        int cmp;
        if (i1 % 3 == 1)
        {
            int a0 = T[i0], a1 = T[i1],
                r0 = (i0 + 1 < n ? rank[i0 + 1] : 0),
                r1 = (i1 + 1 < n ? rank[i1 + 1] : 0);
            cmp = (a1 != a0 ? a1 - a0 : r1 - r0);
        }
        else
        {
            int a0 = T[i0], a1 = T[i1],
                b0 = (i0 + 1 < n ? T[i0 + 1] : 0),
                b1 = (i1 + 1 < n ? T[i1 + 1] : 0),
                r0 = (i0 + 2 < n ? rank[i0 + 2] : 0),
                r1 = (i1 + 2 < n ? rank[i1 + 2] : 0);
            if (a1 != a0)
                cmp = a1 - a0;
            else if (b1 != b0)
                cmp = b1 - b0;
            else
                cmp = r1 - r0;
        }
        if (cmp < 0)
            SA[p++] = i1, p1++;
        else
            SA[p++] = i0, p0++;
    }
    while (p0 < S0sz)
        SA[p++] = SA0[p0++];
    while (p1 < S1sz)
        SA[p++] = SA1[p1++];

    // debug‐print final SA
    // printf("Merged SA:");
    // for (int i = 0; i < n; i++)
    // {
    //     printf(" %d", SA[i]);
    // }
    // printf("\n");
    free(S1);
    free(SA1);
    free(tmp);
    free(rank);
    free(R);
    free(S0);
    free(arr);
    free(SA0);
}

// ----------------------------------------------------------------------
// compute_bwt: unchanged
// ----------------------------------------------------------------------
void compute_bwt_dc3(int *T, int n, int *SA, char *BWT)
{
    for (int i = 0; i < n; i++)
    {
        int pi = SA[i] == 0 ? n - 1 : SA[i] - 1;
        BWT[i] = (char)T[pi];
    }
}

size_t bwt_encode_dc3(const uint8_t *input, size_t len, uint8_t *output)
{
    size_t n = len + 1;
    printf("bwt \n");
    // Build integer text + sentinel
    int *T = malloc(n * sizeof *T);
    int *SA = malloc(n * sizeof *SA);
    for (size_t i = 0; i < len; i++)
        T[i] = input[i];
    T[len] = 0;

    // Build suffix array
    build_dc3_sa(T, (int)n, SA);

    // Produce BWT and record primary
    size_t primary = 0;
    for (size_t i = 0; i < n; i++)
    {
        if (SA[i] == 0)
            primary = i;
        size_t pi = (SA[i] == 0 ? len : SA[i] - 1);
        output[i] = (uint8_t)T[pi];
    }

    free(T);
    free(SA);
    return primary;
}

// Invert a binary BWT of length n (including the 0x00 sentinel).
// Returns a freshly malloc()'d buffer of length n-1 containing the original bytes.
uint8_t *invert_bwt_dc3(const uint8_t *L, size_t n) {
    // 1) Count total occurrences of each byte value
    size_t count[ALPHABET] = {0};
    for (size_t i = 0; i < n; i++) {
        count[L[i]]++;
    }

    // 2) Build C[c] = first index of c in the sorted first column
    size_t C[ALPHABET];
    size_t sum = 0;
    for (int c = 0; c < ALPHABET; c++) {
        C[c] = sum;
        sum += count[c];
    }

    // 3) Build LF mapping via a running occurrence tally
    size_t *occ = calloc(ALPHABET, sizeof *occ);
    size_t *LF  = malloc(n * sizeof *LF);
    for (size_t i = 0; i < n; i++) {
        uint8_t c = L[i];
        LF[i]     = C[c] + occ[c];
        occ[c]++;
    }
    free(occ);

    // 4) Find the row corresponding to the sentinel (0x00) in L
    size_t row = 0;
    while (row < n && L[row] != 0) row++;

    // 5) Walk LF to reconstruct original in reverse order
    uint8_t *tmp = malloc(n);
    for (size_t i = n; i > 0; --i) {
        tmp[i-1] = L[row];
        row       = LF[row];
    }
    free(LF);

    // 6) Drop the trailing sentinel and return
    uint8_t *orig = malloc(n-1);
    // tmp[0..n-2] are the original bytes, tmp[n-1] is the 0x00 sentinel
    memcpy(orig, tmp, n-1);
    free(tmp);
    return orig;
}

uint8_t *bwt_decode_dc3(const uint8_t *bwt_buf, size_t n, size_t primary) {
    // You don’t need the old invert_bwt_dc3 or printf hacks any more:
    (void)primary; // DC3 BWT primary isn’t needed for this LF‐walk style

    return invert_bwt_dc3(bwt_buf, n);
}