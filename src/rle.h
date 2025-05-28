#ifndef RLE_H
#define RLE_H

#include <stddef.h>

/**
 * Run‐length encode `in_buf[0..in_len)`.
 * Returns a malloc’d buffer containing the
 * null‐terminated encoded data, and sets *out_len
 * to the number of bytes (excluding the final '\\0').
 * Caller must free().
 */
char *rle_encode(const char *in_buf, size_t in_len, size_t *out_len);

/**
 * Run‐length decode `in_buf[0..in_len)`.
 * Returns a malloc’d buffer containing the
 * null‐terminated decoded data, and sets *out_len
 * to the number of bytes (excluding the final '\\0').
 * Caller must free().
 */
char *rle_decode(const char *in_buf, size_t in_len, size_t *out_len);

#endif // RLE_H
