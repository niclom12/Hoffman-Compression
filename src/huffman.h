/* huffman.h  –– canonical Huffman for ASCII-127
 *
 * Author: 2025-05-27
 * Public domain / CC0
 */

#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stddef.h>
#include <stdint.h>
#include "fileio.h"
#include "huffsize.h"

extern uint8_t *B; // bit streme buff
extern size_t m;         // number of vailid bits



// this does mean we have a min size for things, becuase we need it to rebuild the tree later
// building the so called canonical codes
// freq  : freq[0..127]  (input)
// len   : len[0..127]   (output) code length for every symbol
// code  : code[0..127]  (output) canonical code, LSB-aligned
size_t huff_build(const int freq[HUFFMAN_SIZE], uint8_t len[HUFFMAN_SIZE], uint32_t code[HUFFMAN_SIZE]);

// encodes using the built canonical codes
void huff_encode(const unsigned char *in, size_t n, const uint8_t len[HUFFMAN_SIZE], const uint32_t code[HUFFMAN_SIZE]);

// decodes, using the bitsream, nbits and the canonoical codes to decode the huffman encoding
size_t huff_decode(const unsigned char *bitstream, size_t nbits, const uint8_t len[HUFFMAN_SIZE], const uint32_t code[HUFFMAN_SIZE], unsigned char *out);

#endif /* HUFFMAN_H */
