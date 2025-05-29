#include "compress.h"
#include "fileio.h"
#include "bwt.h"
#include "rle.h"
#include "huffman.h"
#include "mtf.h"
#include "huffsize.h"
#include "dc3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAGIC "BWH1"

/* helper to write little-endian regardless of host endianness */
static void le64_write(FILE *fp, uint64_t v)
{
    for (int i = 0; i < 8; i++)
        fputc((int)(v >> (i * 8)) & 0xFF, fp);
}
static void le32_write(FILE *fp, uint32_t v)
{
    for (int i = 0; i < 4; i++)
        fputc((int)(v >> (i * 8)) & 0xFF, fp);
}
static uint64_t le64_read(FILE *fp)
{
    uint64_t v = 0;
    for (int i = 0; i < 8; i++) {
        int c = fgetc(fp);
        if (c == EOF) { perror("file"); exit(1); }
        v |= ((uint64_t)c) << (i*8);     /* least-significant first */
    }
    return v;
}

static uint32_t le32_read(FILE *fp)
{
    uint32_t v = 0;
    for (int i = 0; i < 4; i++) {
        int c = fgetc(fp);
        if (c == EOF) { perror("file"); exit(1); }
        v |= ((uint32_t)c) << (i*8);
    }
    return v;
}

// compress
// encode:  TXT > BWT > MTF > RLE > HUFF
int compress(const char *src, const char *dst)
{
    // reading in the file
    long flen;
    uint8_t *raw = read_text_file_to_buffer(src, &flen);
    size_t n;
    char *orig = escape_whitespace(raw, flen, &n);
    free(raw);

    // borrows wheeler transform
    printf("Burrows Wheeler \n");
    uint8_t *bwt_buf = malloc(n+1);
    size_t primary = bwt_encode_dc3(orig, n, bwt_buf);
    for (int i = 0; i < n+1; ++i)
    {
        if (bwt_buf[i] == 0)
            putchar('$');
        else
            putchar(bwt_buf[i]);
    }
    putchar('\n');
    
    // move to front encoding
    printf("Move to Front \n");
    size_t mtf_len;
    uint8_t *mtf_buf = mtf_encode(bwt_buf, n+1, &mtf_len);
    // unsigned char *mtf_buf = mtf_encode(bwt_buf, n + 1, &mtf_len);
    free(bwt_buf); 
    free(orig);
    if (!mtf_buf) { 
        perror("mtf"); 
        return -1; 
    }

    // run length encoding
    size_t rle_len;
    uint8_t *rle_buf = rle_encode(mtf_buf, mtf_len, &rle_len);
    // char *rle_buf = rle_encode((char *)mtf_buf, mtf_len, &rle_len);
    free(mtf_buf);
    if (!rle_buf) { 
        perror("rle_encode");
        return -1; 
    }

    // building huffman and ecoding (entropy encoding)
    int freq[HUFFMAN_SIZE] = {0}; // all frequencies are 0 - since we have normal ascii
    for (size_t i = 0; i < rle_len; i++) {
        freq[rle_buf[i]]++;
    }
        
    uint8_t hlen[HUFFMAN_SIZE] = {0};
    uint32_t hcode[HUFFMAN_SIZE] = {0};
    huff_build(freq, hlen, hcode);

    size_t worst_bits = rle_len * 15;
    extern uint8_t *B; 
    extern size_t m;
    B = calloc((worst_bits + 7) / 8, 1);

    huff_encode((unsigned char *)rle_buf, rle_len, hlen, hcode);
    free(rle_buf);
    size_t byte_len = (m + 7) / 8;

    FILE *fp = fopen(dst, "wb");
    if (!fp)
    {
        perror("fopen");
        return -1;
    }

    fwrite(MAGIC, 1, 4, fp);
    le64_write(fp, (uint64_t)n);
    le64_write(fp, (uint64_t)primary);
    le32_write(fp, (uint32_t)m);
    fwrite(hlen, 1, HUFFMAN_SIZE, fp);
    fwrite(B, 1, byte_len, fp);
    fclose(fp);
    free(B);
    return 0;
}


int decompress(const char *src, const char *dst)
{
    FILE *fp = fopen(src, "rb");
    if (!fp) { 
        perror("fopen"); 
        return -1; 
    }
    
    char tag[5] = {0};
    if (fread(tag, 1, 4, fp) != 4 || strcmp(tag, MAGIC) != 0) {
        fprintf(stderr, "bad file\n");  
        fclose(fp); 
        return -1;
    }
    uint64_t orig_len = le64_read(fp);
    uint64_t primary  = le64_read(fp);
    uint32_t bit_len  = le32_read(fp);
   
    uint8_t  hlen [HUFFMAN_SIZE];
    if (fread(hlen,1,HUFFMAN_SIZE,fp) != HUFFMAN_SIZE) {
        fprintf(stderr,"truncated header\n"); 
        fclose(fp); 
        return -1;
    }
    uint32_t hcode[HUFFMAN_SIZE] = {0};
    {   
        uint32_t bl_count[HUFFMAN_SIZE+1] = {0}, 
        next_code[HUFFMAN_SIZE+1];
        for (int s = 0; s < HUFFMAN_SIZE; ++s) {
            bl_count[ hlen[s] ]++;
        }
        bl_count[0] = 0; 

        uint32_t code = 0; 
        next_code[0] = 0;
        for (int bits = 1; bits <= HUFFMAN_SIZE; ++bits) {
            code = (code + bl_count[bits-1]) << 1;
            next_code[bits] = code;
        }
        for (int s = 0; s < HUFFMAN_SIZE; ++s) {
            if (hlen[s]) {
                hcode[s] = next_code[ hlen[s] ]++;
            }
        }
    }

    size_t byte_len = (bit_len + 7) / 8;
    unsigned char *bitbuf = malloc(byte_len);
    if (!bitbuf) {
        perror("malloc"); 
        fclose(fp); 
        return -1; 
    }
    if (fread(bitbuf, 1, byte_len, fp) != byte_len) {
        fprintf(stderr,"truncated stream\n"); 
        fclose(fp); 
        free(bitbuf); return -1;
    }
    fclose(fp);

    uint8_t *rle_buf = malloc(orig_len+1);
    if (!rle_buf) {
         perror("malloc"); 
         free(bitbuf); return -1; 
    }

    size_t rle_len = huff_decode(bitbuf, bit_len, hlen, hcode, NULL);
    huff_decode(bitbuf, bit_len, hlen, hcode, rle_buf);
    free(bitbuf);

    size_t mtf_len;
    uint8_t *mtf_buf = rle_decode(rle_buf, rle_len, &mtf_len);
    rle_decode(rle_buf, rle_len, &mtf_len);
    free(rle_buf);
    
    if (!mtf_buf) { 
        fprintf(stderr,"RLE decode failed\n"); 
        return -1; 
    }
    
    size_t bwt_len;
    uint8_t *bwt_buf = mtf_decode(mtf_buf, mtf_len, &bwt_len);
    free(mtf_buf);
    if (!bwt_buf) { 
        fprintf(stderr,"MTF decode failed\n"); 
        return -1; 
    }
    
    //uint8_t *bwt_decode_dc3(const uint8_t *bwt_buf, size_t n, size_t primary)
    uint8_t *recovered = bwt_decode_dc3(bwt_buf, bwt_len, (size_t)primary);
    char *final = unescape_whitespace((char*)recovered, bwt_len, &primary);
    
    free(bwt_buf);
    free(recovered);

    FILE *out = fopen(dst, "wb");
    if (!out) { 
        perror("out"); 
        free(recovered); 
        return -1; 
    }

    fwrite(final, 1, orig_len, out);
    fclose(out);
    free(final);
    return 0;
}
