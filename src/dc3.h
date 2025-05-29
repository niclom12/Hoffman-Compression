#ifndef DC3_H
#define DC3_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void compute_bwt_dc3(int *T, int n, int *SA, char *BWT);
size_t bwt_encode_dc3(const uint8_t *input, size_t len, uint8_t *output);
uint8_t *bwt_decode_dc3(const uint8_t *bwt_buf, size_t n, size_t primary);
#endif