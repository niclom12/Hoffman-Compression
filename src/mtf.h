// move to front logical abstractions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifndef MTF_H
#define MTF_H

// This performs the encoding and retuns the integer array
uint8_t *mtf_encode(const uint8_t *in,
                                 size_t n, size_t *out_len);

// Decodes the interger array into original character string
uint8_t *mtf_decode(const uint8_t*indices,
                                 size_t n, size_t *out_len);

#endif