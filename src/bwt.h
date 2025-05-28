#ifndef BWT_H
#define BWT_H

#include <stddef.h>
#include <stdint.h>

/**
 * Perform the Burrows–Wheeler transform on an input buffer.
 *
 * @param input     Pointer to the input bytes (length = len).
 * @param len       Number of bytes in input.
 * @param bwt_out   Buffer of length len+1 to receive the BWT output.
 *                  (You must allocate this before calling.)
 * @return          The primary index in the sorted rotations
 *                  (i.e. the row corresponding to the original text).
 */
size_t bwt_encode(const uint8_t *input, size_t len, uint8_t *bwt_out);

/**
 * Invert a Burrows–Wheeler transform.
 *
 * @param bwt               Pointer to the BWT data (length = len).
 * @param len               Number of bytes in bwt (including the sentinel).
 * @param primary_index     The index returned by bwt_encode.
 * @return                  Newly malloc’d buffer of length (len) bytes,
 *                          containing the original data _including_
 *                          the trailing '\0' sentinel. You can drop
 *                          that sentinel if you only want the original
 *                          len−1 bytes.
 */
uint8_t *bwt_decode(const uint8_t *bwt, size_t len, size_t primary_index);

#endif // BWT_H
