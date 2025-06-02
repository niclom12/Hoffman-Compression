#ifndef BWT_H
#define BWT_H

#include <stddef.h>
#include <stdint.h>

size_t bwt_encode(const uint8_t *input, size_t len, uint8_t *bwt_out);

size_t bwt_encode_count_sort(const uint8_t *T, size_t n, uint8_t *L);

uint8_t *bwt_decode(const uint8_t *bwt, size_t len, size_t primary_index);

void counting_sort(int *sa, int *rank, int n, int k, int *tmp);

#endif 
