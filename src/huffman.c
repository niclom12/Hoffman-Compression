#include "huffman.h"
#include "heap.h"
#include <stdlib.h>
#include <string.h>
typedef struct
{
    int node;
    int depth;
} NodePosition;

unsigned char *B = NULL;
size_t m = 0;

// Forward declarations
static int init_leaves(const int freq[], struct node nodes[], int heap[], int *n_nodes);
static int build_tree(struct node nodes[], int heap[], int n_leaves, int *n_nodes);
static void compute_lengths(const struct node nodes[], int root, uint8_t length[]);
static void generate_canonical(const uint8_t length[], uint32_t canon[]);

size_t huff_build(const int freq[], uint8_t length[], uint32_t canon[])
{
    // contains all the leaves of the tree, set to the number of symbols in our alphabet i.e. HUFF_MAX
    struct node nodes[HUFF_MAX_NODES];
    // min heap of nodes by frequency
    int heap[HUFF_MAX_NODES];
    int n_nodes = 0;

    // init a node for every symbol that has a non zero frequency
    int n_leaves = init_leaves(freq, nodes, heap, &n_nodes);
    if (n_leaves == 0) // mmmmm no nodes???
        return 0;
    // we can fast track this if we only have one
    if (n_leaves == 1)
    {
        length[nodes[heap[0]].sym] = 1;
        canon[nodes[heap[0]].sym] = 0;
        return 1;
    }
    // merge nodes to build huff tree, and get the root
    int root = build_tree(nodes, heap, n_leaves, &n_nodes);
    // compute the length with dfs for each of da symbobaclaats
    compute_lengths(nodes, root, length);
    // convert symbols into cannon
    generate_canonical(length, canon);

    return (size_t)n_leaves; // number of leaves / symbols in the encoding
}

static int init_leaves(const int freq[], struct node nodes[], int heap[], int *n_nodes)
{
    int n_leaves = 0;
    // for all the symbols
    for (int s = 0; s < HUFFMAN_SIZE; ++s)
    {
        if (freq[s] == 0) // skip zero freq
            continue;
        // inti a leaf, by making its left and right -1 indicating its got no kids and assign freq
        nodes[*n_nodes] = (struct node){.freq = freq[s], .left = -1, .right = -1, .sym = (uint8_t)s};
        // add index to the heap
        heap[n_leaves] = *n_nodes;
        // swim the new node up
        swim(heap, n_leaves, nodes);
        ++n_leaves;
        ++(*n_nodes);
    }
    return n_leaves;
}

static int build_tree(struct node nodes[], int heap[], int heap_sz, int *n_nodes)
{
    while (heap_sz > 1) // while we've still got nodes in the heap
    {
        // get the smallest freq, replace root with largest, sink em down
        int a = heap[0];
        heap[0] = heap[--heap_sz];
        sink(heap, heap_sz, 0, nodes);
        // same
        int b = heap[0];
        heap[0] = heap[--heap_sz];
        sink(heap, heap_sz, 0, nodes);
        // now we merge these two nodes by creating a parent that reps them
        nodes[*n_nodes] = (struct node){.freq = nodes[a].freq + nodes[b].freq, .left = a, .right = b};
        heap[heap_sz] = *n_nodes;   // insert back
        swim(heap, heap_sz, nodes); // swim forest
        // two steps forward one step back
        ++heap_sz;
        ++(*n_nodes);
    }
    return heap[0];
}

static void compute_lengths(const struct node nodes[], int root, uint8_t length[])
{
    // simple stack, node index and depth
    NodePosition stack[HUFF_MAX_NODES];
    // stack counter
    int sp = 0;
    // root on stack
    stack[sp++] = (NodePosition){root, 0};
    while (sp) // while we've still got notes on da stack
    {
        // take it off the satck
        NodePosition cur = stack[--sp];
        // no child
        if (nodes[cur.node].left == -1)
        {
            // gets a code length equal to its depth in the tree
            length[nodes[cur.node].sym] = (uint8_t)cur.depth;
        }
        else
        {
            // got kids so we put em on the stack
            stack[sp++] = (NodePosition){nodes[cur.node].right, cur.depth + 1};
            stack[sp++] = (NodePosition){nodes[cur.node].left, cur.depth + 1};
        }
    }
}

// cannon huffman coding is a way of assigning bit patterns to symbols based on their lengths, mininimzing leng
static void generate_canonical(const uint8_t length[], uint32_t canon[])
{
    // number of symbols that have a code length of bits
    int bit_len_code[HUFFMAN_SIZE + 1] = {0};
    //  next available canonical code for code length bits
    uint32_t next_code[HUFFMAN_SIZE + 1] = {0};
    // increament the number of symbols with a particular bit length -- counting thre occurances of symbol having numb bits
    for (int s = 0; s < HUFFMAN_SIZE; ++s)
        bit_len_code[length[s]]++;

    // compute first cannon codes for each bit len
    uint32_t code = 0;
    for (int bits = 1; bits <= HUFFMAN_SIZE; ++bits)
    {
        // code it = to previous code + previous count and then shifting left to increase bit length
        code = (code + bit_len_code[bits - 1]) << 1; // prefix freeing it up like a mf
        next_code[bits] = code;
    }
    // assign the codes to the symbols
    for (int s = 0; s < HUFFMAN_SIZE; ++s)
    {
        if (length[s])
        {
            canon[s] = next_code[length[s]]++; // next symbol of that length gets a new code
        }
    }
}

// for writting the bit stream
static void write_bits(size_t *bytepos, int *bitpos, uint32_t code, uint8_t len)
{
    // loop over ther bits from Most significant to least
    for (int i = len - 1; i >= 0; --i)
    {
        // checking the ith bit
        if (code & (1u << i))
        {
            // set that bit in the buffer  and align the bugger
            B[*bytepos] |= 1u << (7 - *bitpos);
        }
        // full byte
        if (++(*bitpos) == 8)
        {
            // reset bit pos in new byte
            *bitpos = 0;
            // increment the byte position
            ++(*bytepos);
        }
    }
}

void huff_encode(const unsigned char *in, size_t n, const uint8_t len[], const uint32_t code[])
{
    // finding maximum code length across symbols so we can allocate enough spcae in the buffer
    uint8_t max_len = 0;
    for (int s = 0; s < HUFFMAN_SIZE; ++s)
    {
        if (len[s] > max_len)
        {
            max_len = len[s];
        }
    }
    // this is the worst case, where each symbol has the max number of bits 
    size_t bytes_needed = (n * max_len + 7) / 8;
    memset(B, 0, bytes_needed);
    // position trackers 
    size_t bytepos = 0;
    int bitpos = 0;
    // for every symbol add it to the byte stream 
    for (size_t i = 0; i < n; ++i)
    {
        unsigned char s = in[i];
        // look up the huffman codes for each of the symbols and write them accordingly 
        write_bits(&bytepos, &bitpos, code[s], len[s]);
    }
    // total number of bits written --. this also gets written thats why we extern it 
    m = (bytepos << 3) + bitpos;
}

size_t huff_decode(const unsigned char *bits, size_t nbits, const uint8_t len[], const uint32_t code[], unsigned char *out)
{
    // initial decoding vars
    uint32_t cur = 0;
    int clen = 0;
    size_t o = 0;
    // min and max codes for a given bit length
    uint32_t min_code[HUFFMAN_SIZE + 1];
    uint32_t max_code[HUFFMAN_SIZE + 1];
    // index where codes begin
    int sym_base[HUFFMAN_SIZE + 1];
    // adding a sort of sentinel so we can calc the mins easily 
    memset(min_code, 0xFF, sizeof(min_code));

    // builds the array of the symbols in order of the increasing code lengths
    int order[HUFFMAN_SIZE];
    int k = 0;
    for (int l = 1; l <= HUFFMAN_SIZE; ++l)
        for (int s = 0; s < HUFFMAN_SIZE; ++s)
            if (len[s] == l)
                order[k++] = s;


    // now we look up the symbols
    int last_len = 0;
    // so for each symbol
    for (int i = 0; i < k; ++i)
    {
        int s = order[i];
        // if its the first symbol of its length
        if (len[s] != last_len)
        {   
            // set the min and max to that symbol's length
            min_code[len[s]] = max_code[len[s]] = code[s];
            sym_base[len[s]] = i;
            last_len = len[s];
        }
        else
        {
            // seen this length before so we can increase max_code to be the length of the new code 
            max_code[len[s]] = code[s];
        }
    }
    // decode bit by bit 
    for (size_t b = 0; b < nbits; ++b)
    {
        // for each of the bits in the encoding stream
        // shit cur over for the new bit, extract the new bit at the postition (7 - (b & 7)))) 
        // we're moving from MSB to LSB 
        cur = (cur << 1) | ((bits[b >> 3] >> (7 - (b & 7))) & 1u);
        ++clen; // increament the length of the code 
        // if the length is within bounds, and it matches a code (greater than min and less than max)
        if (clen <= HUFFMAN_SIZE && cur >= min_code[clen] && cur <= max_code[clen])
        {
            // find the index that the code refers to from order[]
            int idx = sym_base[clen] + (int)(cur - min_code[clen]);
            if (out) {
                // if we have to output, then add that symbol to the output
                out[o] = (unsigned char)order[idx];
            }
            // reset the position vars for the next symbol decode.
            ++o;
            cur = 0;
            clen = 0;
        }
    }
    return o;
}
