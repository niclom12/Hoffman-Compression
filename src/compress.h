#ifndef BWT_HUFF_COMPRESS_H
#define BWT_HUFF_COMPRESS_H

// compression 
int compress(const char *src_path, const char *dst_path);
// decompression
int decompress(const char *src_path, const char *dst_path);

#endif