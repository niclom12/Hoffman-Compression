#ifndef BWT_HUFF_COMPRESS_H
#define BWT_HUFF_COMPRESS_H

// compression 
int compress(const char *src, const char *dst, int use_bwt, int use_mtf, int use_rle);
// decompression
int decompress(const char *src, const char *dst, int use_bwt, int use_mtf, int use_rle);

#endif