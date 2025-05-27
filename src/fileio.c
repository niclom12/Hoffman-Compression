/**
 * This is the code used to read in the files and break them up into their tokens/symbles. Plan to extend to other file types
 */
#include "fileio.h"
#include <stdlib.h>

// Bit array for encoded / decoded version of buffer
Boolean *B;
// Index to where we are in encoded / decoded routine
size_t m = 0;

/**
 * Count character frequencies in a text file
 */
int read_frequencies(const char *filename, int *freq) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return -1;
    } 

    // Initialize frequency array to all zeros
    memset(freq, 0, HUFFMAN_SIZE * sizeof(int));

    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        freq[(__uint8_t)ch]++;
    }

    fclose(fp);
    return 0;
}

// Reads entire file into a buffer. Gonna use this for iteration when generating the hofman codes after the frequencies are calculated
char* read_text_file_to_buffer(const char *filename, long *length) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    *length = ftell(fp);
    rewind(fp);

    char *buffer = (char *)malloc(*length + 1);

    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    size_t ret = fread(buffer, sizeof(char), *length, fp);
    // if haven't read full buffer for whatever reason
    if (ret < *length) {
        return NULL;
    }
    buffer[*length] = '\0';
    fclose(fp);
    return buffer;
}

// Write binary data to output file. For writing back compressed data
int write_binary_file(const char *filename, const __uint8_t *data, long length) {
    FILE *fp = fopen(filename, "wb");

    if (!fp) {
        return -1;
    }

    fwrite(data, sizeof(__uint8_t), length, fp);
    fclose(fp);
    return 0;
}

// Write plain text data (decompressed)
int write_text_file(const char *filename, const char *text, long length) {
    FILE *fp = fopen(filename, "w");

    if (!fp) {
        return -1;
    }

    fwrite(text, sizeof(char), length, fp);
    fclose(fp);
    return 0;
}

// Exact replica (but in C) of Algorithmns by JeffE in Section 4.4
// But its zero indexed. Returns index to root. 
// * Tested on input from textbook and is correct.
int buildHuffman(int *freq, int *left, int *right, int *parent) {
    MinHeap heap;
    minheap_init(&heap);
    size_t i = 0;
    // No initialization of f[i] needed for >= MAX_SYMBOLS - 1 as filled as it is built below
    for (i = 0; i < MAX_SYMBOLS; i++)
    {
        left[i] = right[i] = 0;
        if (freq[i] == 0) continue;
        minheap_insert(&heap, create_node(i, freq[i]));
    }
    // Above tested (printed out against example).  
    // If there is only 1 element left, then its just the root which is fine.
    for (; i < HUFFMAN_SIZE && heap.size > 1; i++)
    {
        Node *x = minheap_extract_min(&heap);
        Node *y = minheap_extract_min(&heap);
        freq[i] = freq[x->ch] + freq[y->ch];
        if (DEBUG_EXTRA) printf("i: %ld, x: '%c', y: '%c', freq: %d\n", i, x->ch, y->ch, freq[i]);
        // ! Note i can easily go out of range of (printable) character limit. 
        minheap_insert(&heap, create_node(i, freq[i]));
        left[i] = x->ch; right[i] = y->ch;
        parent[x->ch] = parent[y->ch] = i;
        free(x);
        free(y);
    }
    // Memory management
    if (heap.size > 0) {
        free(minheap_extract_min(&heap));
    }
    // Root
    parent[--i] = 0;
    return i;
}

// As in textbook
void huffmanEncode(char *buffer, long k, int *left, int *parent, int root) {
    m = 0;
    if (DEBUG) printf("Length: %ld\n", k);
    for (size_t i = 0; i < k; i++)
    {
        huffmanEncodeOne(buffer[i], left, parent, root);
    }
}

void huffmanEncodeOne(int x, int *left, int *parent, int root) {
    if (x < root) {
        huffmanEncodeOne(parent[x], left, parent, root);
        if (x == left[parent[x]]) {
            B[m] = FALSE;
        } else {
            B[m] = TRUE;
        }
        m++;
    }
}

// As in textbook
// ! We would not usually have length so this needs to be adjusted
// ! It also assumes we have left, right and root (which we need to explicitly add in encoding I believe)
char* huffmanDecode(Boolean *B, int *left, int *right, int root, long length) {
    int k = 0;
    int v = root;
    char *A = (char *) malloc(sizeof(char) * length);
    for (size_t i = 0; i < m; i++)
    {
        if (B[i] == 0) {
            v = left[v];
        } else {
            v = right[v];
        }
        if (left[v] == 0) {
            A[k++] = v;
            v = root;
        }
    }
    A[k] = '\0';
    return A;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int freq[HUFFMAN_SIZE];
    if (read_frequencies(argv[1], freq) != 0) {
        printf("Failed to open file: %s\n", argv[1]);
        return 1;
    }

    printf("Character Frequencies in '%s':\n", argv[1]);
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (freq[i] > 0) {
            if (i == '\n') {
                printf("'\\n' : %d\n", freq[i]);
            } else if (i == ' ') {
                printf("' '  : %d\n", freq[i]);
            } else {
                printf("'%c'  : %d\n", i, freq[i]);
            }
        }
    }

    // Perform unit test for Min Heap
    // printf("\nPerforming Unit tests on Min heap\n");
    // unit_test();

    // Build HuffMan code and print out
    // TODO: Rework this size, because its uneccessary, we only need enough for
    // each entry that has a frequency to be combined, thats the n. 
    // Looks like implementation needs to change, maybe use Canonical Huffman Codes.
    // TODO: Fix using int and char interchangeably (its fine when we just go to I assume longs for everything)
    int left[HUFFMAN_SIZE];
    int right[HUFFMAN_SIZE];
    int parent[HUFFMAN_SIZE];
    int root = buildHuffman(freq, left, right, parent);
    // Number of characters in buffer
    long k;
    char *buffer = read_text_file_to_buffer(argv[1], &k);
    // Using max size. k characters * biggest space a code would take
    B = (Boolean *) malloc(sizeof(Boolean) * k * ceil(log2(root)));
    huffmanEncode(buffer, k, left, parent, root);

    if (DEBUG) {
        for (size_t i = 0; i < m; i++)
        {
            printf("%d", B[i]);
        }
        printf("\n");
    }

    char *A = huffmanDecode(B, left, right, root, k);
    if (DEBUG)
    {
        for (int i = 0; i < k; i++)
        {
            printf("%c", A[i]);
        }
        printf("\n");
    }

    // Memory management
    free(buffer);
    free(B);
    free(A);

    return 0;
}