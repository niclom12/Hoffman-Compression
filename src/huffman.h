#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stddef.h>
#include <stdint.h>
#include "fileio.h"
#include "huffsize.h"

extern unsigned char *B; // bit streme buff
extern size_t m; // number of vailid bits


size_t huff_build(const int freq[], uint8_t len[], uint32_t code[]);

// encodes using the built canonical codes
void huff_encode(const unsigned char *in, size_t n, const uint8_t len[], const uint32_t code[]);

// decodes, using the bitsream, nbits and the canonoical codes to decode the huffman encoding
size_t huff_decode(const unsigned char *bitstream, size_t nbits, const uint8_t len[], const uint32_t code[], unsigned char *out);

#endif 
