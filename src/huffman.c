#include "huffman.h"
#include <string.h>   
#include <stdlib.h>   
#include "heap.h"      

typedef struct { 
    int node; 
    int depth; 
} NodePosition;

unsigned char *B = NULL;   
size_t m = 0;    

static int  init_leaves   (const int freq[HUFFMAN_SIZE], struct node nodes[], int heap[], int *n_nodes);

static int  build_tree    (struct node nodes[], int heap[], int n_leaves, int *n_nodes);

static void compute_lengths(const struct node nodes[], int root, uint8_t length[HUFFMAN_SIZE]);

static void generate_canonical(const uint8_t  length[HUFFMAN_SIZE], uint32_t canon [HUFFMAN_SIZE]);

size_t huff_build(const int freq  [HUFFMAN_SIZE], uint8_t length[HUFFMAN_SIZE], uint32_t canon [HUFFMAN_SIZE])
{
    struct node nodes[HUFF_MAX_NODES];
    int         heap [HUFF_MAX_NODES];
    int         n_nodes = 0;

    // initialize the leaves of the tree
    int n_leaves = init_leaves(freq, nodes, heap, &n_nodes);
    if (n_leaves == 0) return 0;            /* empty alphabet */

    // when theres only onse symbol we must differentiate between it and the sentinel (in terms of frequency)
    if (n_leaves == 1) {
        length[nodes[heap[0]].sym] = 1;
        canon [nodes[heap[0]].sym] = 0;
        return 1;
    }

    // build the huffman tree and get the root
    int root = build_tree(nodes, heap, n_leaves, &n_nodes);

    // compute the code lengths to get to specific roots 
    compute_lengths(nodes, root, length);

    // turn the lengths into canonical codes so that it  can be rebuild in decoding 
    generate_canonical(length, canon);

    return (size_t)n_leaves;
}

// initializes the nodes and the min heap
static int init_leaves(const int freq[HUFFMAN_SIZE], struct node nodes[], int heap[], int *n_nodes)
{
    int n_leaves = 0;
    for (int s = 0; s < HUFFMAN_SIZE; ++s) {
        if (freq[s] == 0) continue;
        // felt good with the pass by vector and deref for the indexing
        nodes[*n_nodes].freq = freq[s]; // assing the node at the current number of node's index the corresponding frequency
        nodes[*n_nodes].left = nodes[*n_nodes].right = -1; // make its children empty 
        nodes[*n_nodes].sym = (uint8_t)s; // assign it the symbol of the corresponding index
        heap[n_leaves] = *n_nodes; // modify the heap
        swim(heap, n_leaves, nodes); // swim the node up to the right place wrt. its frequency
        ++n_leaves; 
        ++(*n_nodes);
    }
    return n_leaves;
}

// builds the tree (entropy encoding step where we merge the symbols according to frequency)
static int build_tree(struct node nodes[], int heap[], int heap_sz, int *n_nodes)
{
    while (heap_sz > 1) {
        // removing the heads (min frequency)
        int a = heap[0]; 
        heap[0] = heap[--heap_sz];
        sink(heap, heap_sz, 0, nodes);

        int b = heap[0]; 
        heap[0] = heap[--heap_sz];
        sink(heap, heap_sz, 0, nodes);

        // now we are combining the symbols and their frequencies 
        nodes[*n_nodes].freq  = nodes[a].freq + nodes[b].freq;
        nodes[*n_nodes].left  = a;
        nodes[*n_nodes].right = b;

        heap[heap_sz] = *n_nodes;
        swim(heap, heap_sz, nodes); // and we swim that node to its correct point

        ++heap_sz;
        ++(*n_nodes);
    }
    return heap[0];
}

// dfs to get the values of the lengths to each of the symbols
static void compute_lengths(const struct node nodes[], int root, uint8_t length[HUFFMAN_SIZE])
{
    NodePosition stack[HUFF_MAX_NODES];
    int sp = 0;

    stack[sp++] = (NodePosition){ root, 0 };

    while (sp) {
        NodePosition cur = stack[--sp];

        if (nodes[cur.node].left == -1) { // we have no children (-1) so it must be a leaf node
            length[nodes[cur.node].sym] = (uint8_t)cur.depth;
        } else { // else its an internal node so we keep incrementing depth                                  
            stack[sp++] = (NodePosition){ nodes[cur.node].right,
                                       cur.depth + 1 };
            stack[sp++] = (NodePosition){ nodes[cur.node].left,
                                       cur.depth + 1 };
        }
    }
}

// canonical codes for the 'header' so that it can be rebuilt later
static void generate_canonical(const uint8_t length[HUFFMAN_SIZE], uint32_t canon [HUFFMAN_SIZE])
{
    int bl_count [HUFFMAN_SIZE + 1] = {0};
    uint32_t next_code[HUFFMAN_SIZE + 1];

    // for each of the symbols 
    for (int s = 0; s < HUFFMAN_SIZE; ++s) {
        if (length[s]) {
            bl_count[length[s]]++;
        }
    }

    // assign first code for each length (DEFLATE spec 3.2.2)
    next_code[0] = 0;
    uint32_t code = 0;
    for (int bits = 1; bits <= HUFFMAN_SIZE; ++bits) {
        code = (code + bl_count[bits - 1]) << 1;
        next_code[bits] = code;
    }

    // final per-symbol canonical code 
    for (int s = 0; s < HUFFMAN_SIZE; ++s) {
        if (length[s]) {
            canon[s] = next_code[length[s]]++;
        }   
    }
}
                    
static void write_bits(size_t *bytepos, int *bitpos, uint32_t code, uint8_t len)
{
    for (int i = len - 1; i >= 0; --i) {
        if (code & (1u << i)) {
            B[*bytepos] |= 1u << (7 - *bitpos);
        }
        if (++(*bitpos) == 8) { 
            *bitpos = 0; ++(*bytepos); 
        }
    }
}

void huff_encode(const unsigned char *in, size_t n, const uint8_t len [HUFFMAN_SIZE], const uint32_t code[HUFFMAN_SIZE])
{   
    uint8_t max_len = 0;
    for (int s = 0; s < HUFFMAN_SIZE; ++s) {
        if (len[s] > max_len) {
            max_len = len[s]; // max length down the 'tree'
        }
    }

    size_t bytes_needed = (n * max_len + 7) / 8;
    memset(B, 0, bytes_needed);

    size_t bytepos = 0; 
    int bitpos = 0;

    for (size_t i = 0; i < n; ++i) {
        unsigned char s = in[i];
        write_bits(&bytepos, &bitpos, code[s], len[s]);
    }
    m = (bytepos << 3) + bitpos;
}

size_t huff_decode(const unsigned char *bits, size_t nbits, const uint8_t  len [HUFFMAN_SIZE], const uint32_t code[HUFFMAN_SIZE], unsigned char *out)
{
    uint32_t min_code[HUFFMAN_SIZE+1], max_code[HUFFMAN_SIZE+1];
    int sym_base[HUFFMAN_SIZE+1];
    memset(min_code, 0xFF, sizeof(min_code));

    int order[HUFFMAN_SIZE]; int k=0;
    for (int l=1; l<=HUFFMAN_SIZE; ++l) {
        for (int s=0; s<HUFFMAN_SIZE; ++s) {
            if (len[s]==l) order[k++]=s;
        }
    }
        
    int last_len = 0;
    for (int i=0; i<k; ++i) {
        int s = order[i];
        if (len[s]!=last_len) {
            min_code[len[s]] = code[s];
            max_code[len[s]] = code[s];
            sym_base[len[s]] = i;
            last_len = len[s];
        } else {
            max_code[len[s]] = code[s];
        }
    }
    
    uint32_t cur = 0;
    int clen = 0; 
    size_t o = 0;

    for (size_t b = 0; b < nbits; ++b) {
        cur  = (cur << 1) | ((bits[b >> 3] >> (7 - (b & 7))) & 1u);
        ++clen;
        if (clen <= HUFFMAN_SIZE && cur >= min_code[clen] && cur <= max_code[clen]) {
            int idx = sym_base[clen] + (int)(cur - min_code[clen]);
            if (out) {
                out[o] = (unsigned char)order[idx];
            }                  
            ++o;                          
            cur  = 0;
            clen = 0;
        }
    }
    return o;     
}
