#include "compress.h"
#include "fileio.h"
#include "bwt.h"
#include "rle.h"
#include "huffman.h"
#include "mtf.h"
#include "huffsize.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAGIC "BWH1"

/* Simple CRC-32 (poly = 0xEDB88320, same as zlib) ------------------------- */
#include <stdint.h>

uint32_t crc32(const void *data, size_t len)
{
    static uint32_t table[256];
    static int      have_table = 0;

    if (!have_table) {                               /* build once          */
        for (int i = 0; i < 256; ++i) {
            uint32_t crc = (uint32_t)i;
            for (int k = 0; k < 8; ++k)
                crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320u : crc >> 1;
            table[i] = crc;
        }
        have_table = 1;
    }

    uint32_t crc = 0xFFFFFFFFu;
    const unsigned char *p = data;
    while (len--)
        crc = table[(crc ^ *p++) & 0xFFu] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}


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
    char *orig = read_text_file_to_buffer(src, &flen);
    if (!orig)
    {
        perror("read");
        return -1;
    }
    size_t n = (size_t)flen;

    // borrows wheeler transform
    uint8_t *bwt_buf = malloc(n + 1);
    size_t primary = bwt_encode2((uint8_t *)orig, n, bwt_buf);
    printf("After BWT encode : %08x (%zu bytes)\n", crc32(bwt_buf, n+1), n+1);
    
    // move to front encoding
    size_t mtf_len;
    unsigned char *mtf_buf = mtf_encode(bwt_buf, n + 1, &mtf_len);
    free(bwt_buf); free(orig);
    if (!mtf_buf) { perror("mtf"); return -1; }
    printf("After MTF encode : %08x (%zu bytes)\n", crc32(mtf_buf, mtf_len), mtf_len);

    // run length encoding
    size_t rle_len;
    char *rle_buf = rle_encode((char *)mtf_buf, mtf_len, &rle_len);
    free(mtf_buf);
    if (!rle_buf) { perror("rle_encode"); return -1; }

    printf("After RLE encode : %08x (%zu bytes)\n", crc32(rle_buf, rle_len), rle_len);


    // building huffman and ecoding (entropy encoding)
    int freq[HUFFMAN_SIZE] = {0}; // all frequencies are 0 - since we have normal ascii
    for (size_t i = 0; i < rle_len; i++) {
        freq[(unsigned char)rle_buf[i]]++;
    }
        
    uint8_t hlen[HUFFMAN_SIZE] = {0};
    uint32_t hcode[HUFFMAN_SIZE] = {0};
    huff_build(freq, hlen, hcode);

    
    uint8_t max_len = 0;
    for (int s = 0; s < HUFFMAN_SIZE; ++s)
        if (hlen[s] > max_len) max_len = hlen[s];
    size_t worst_bits = rle_len * max_len;

    extern unsigned char *B; 
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
    // read in the compressed file
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

    

    size_t rle_len = huff_decode(bitbuf, bit_len, hlen, hcode, NULL);
    char *rle_buf  = malloc(rle_len); 
    huff_decode(bitbuf, bit_len, hlen, hcode, (unsigned char*)rle_buf);
    free(bitbuf);
    
    size_t mtf_len;
    unsigned char *mtf_buf = (unsigned char*) 
    rle_decode(rle_buf, rle_len, &mtf_len);
    
    printf("After RLE decode : %08x (%zu bytes)\n", crc32(rle_buf, rle_len), rle_len);
    free(rle_buf);

    if (!mtf_buf) { 
        fprintf(stderr,"RLE decode failed\n"); 
        return -1; 
    }
    
    size_t bwt_len;
    unsigned char *bwt_buf = mtf_decode(mtf_buf, mtf_len, &bwt_len);
    free(mtf_buf);
    if (!bwt_buf) { 
        fprintf(stderr,"MTF decode failed\n"); 
        return -1; 
    }

    printf("After MTF decode : %08x (%zu bytes)\n", crc32(mtf_buf, mtf_len), mtf_len);

    
    uint8_t *recovered = bwt_decode_dc3(bwt_buf, bwt_len, (size_t)primary);
    // memmove(recovered, recovered + 1, orig_len);

    printf("After BWT decode : %08x (%zu bytes)\n", crc32(bwt_buf, bwt_len), bwt_len);

    free(bwt_buf);
    FILE *out = fopen(dst, "wb");
    if (!out) { 
        perror("out"); 
        free(recovered); 
        return -1; 
    }

    fwrite(recovered, 1, orig_len, out);
    fclose(out);
    free(recovered);
    return 0;
}
