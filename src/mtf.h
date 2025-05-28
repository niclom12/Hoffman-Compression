// move to front logical abstractions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifndef MTF_H
#define MTF_H

// This performs the encoding and retuns the integer array
unsigned char *mtf_encode(const unsigned char *in,
                                 size_t n, size_t *out_len);

// Decodes the interger array into original character string
unsigned char *mtf_decode(const unsigned char *indices,
                                 size_t n, size_t *out_len);

#endif