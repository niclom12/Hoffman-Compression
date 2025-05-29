#ifndef RLE_H
#define RLE_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Run‐length encode `in_buf[0..in_len)`.
 * Returns a malloc’d buffer containing the
 * null‐terminated encoded data, and sets *out_len
 * to the number of bytes (excluding the final '\\0').
 * Caller must free().
 */
uint8_t *rle_encode(const uint8_t *in_buf, size_t in_len, size_t *out_len);

/**
 * Run‐length decode `in_buf[0..in_len)`.
 * Returns a malloc’d buffer containing the
 * null‐terminated decoded data, and sets *out_len
 * to the number of bytes (excluding the final '\\0').
 * Caller must free().
 */
uint8_t *rle_decode(const uint8_t *in_buf, size_t in_len, size_t *out_len);

#endif // RLE_H
