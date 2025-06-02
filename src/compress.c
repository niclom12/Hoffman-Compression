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

#define START "S" // for valitatin
#define FLAG_BWT 1
#define FLAG_MTF 2
#define FLAG_RLE 4

static void le64_write(FILE *fp, uint64_t v);
static void le32_write(FILE *fp, uint32_t v);
static uint64_t le64_read(FILE *fp);
static uint32_t le32_read(FILE *fp);
static void replace_buffer(unsigned char **buf, unsigned char *new_buf, unsigned char *original);
static uint8_t get_max_len(const uint8_t *hlen);

// Compression
int compress(const char *src, const char *dst, int use_bwt, int use_mtf, int use_rle)
{
    // Flags for indicating the pipeline
    uint8_t flags = (use_bwt ? FLAG_BWT : 0) | (use_mtf ? FLAG_MTF : 0) | (use_rle ? FLAG_RLE : 0);
    // b stream
    extern unsigned char *B;
    // number of encoded bits
    extern size_t m;

    long flen;
    char *orig = read_text_file_to_buffer(src, &flen);
    if (!orig)
    {
        perror("read");
        return -1;
    }

    size_t len = (size_t)flen;
    unsigned char *buf = (unsigned char *)orig;
    size_t primary = 0;

    // Burrows Wheeler Transform
    if (use_bwt)
    {
        uint8_t *bwt_buf = malloc(len + 1);
        primary = bwt_encode_count_sort(buf, len, bwt_buf);
        replace_buffer(&buf, bwt_buf, (unsigned char *)orig);
        len += 1;
    }

    // Move to Front Encoding
    if (use_mtf)
    {
        size_t mtf_len;
        unsigned char *mtf_buf = mtf_encode(buf, len, &mtf_len);
        replace_buffer(&buf, mtf_buf, (unsigned char *)orig);
        len = mtf_len;
    }

    // Run Length Encoding
    if (use_rle)
    {
        size_t rle_len;
        unsigned char *rle_buf = (unsigned char *)rle_encode((char *)buf, len, &rle_len);
        replace_buffer(&buf, rle_buf, (unsigned char *)orig);
        len = rle_len;
    }

    // Calculate symbol frequencies
    int freq[HUFFMAN_SIZE] = {0};
    for (size_t i = 0; i < len; i++)
        freq[buf[i]]++;
    // length in bits of the huffman code for symbol i.
    uint8_t length_of_symbol[HUFFMAN_SIZE] = {0};
    // canonical huffman code for symbol i
    uint32_t huff_code[HUFFMAN_SIZE] = {0};
    // build huffman
    huff_build(freq, length_of_symbol, huff_code);
    // longest bit rep of the symbols
    uint8_t max_len = get_max_len(length_of_symbol);
    // making sure we allocate enough space
    size_t worst_bits = len * max_len;
    // byte-aligned buffer to store the encoded bitstream & rounds up
    B = calloc((worst_bits + 7) / 8, 1);
    huff_encode(buf, len, length_of_symbol, huff_code);
    free(buf);

    // calcs how many bytes of encoded data are in B
    size_t byte_len = (m + 7) / 8;
    FILE *fp = fopen(dst, "wb");
    if (!fp)
    {
        perror("fopen");
        return -1;
    }
    // write the start
    fwrite(START, 1, 4, fp);
    // wtrites the flags
    fwrite(&flags, 1, 1, fp);
    // file length
    le64_write(fp, (uint64_t)flen);
    // primary index of burrows wheeler
    le64_write(fp, (uint64_t)primary);
    // length of encoded bits
    le32_write(fp, (uint32_t)m);
    // huffman header which has the cannon codes and the bit lengths for the symbols
    fwrite(length_of_symbol, 1, HUFFMAN_SIZE, fp);
    // writes the actual binary data
    fwrite(B, 1, byte_len, fp);

    fclose(fp);
    free(B);
    return 0;
}

// Decompression
int decompress(const char *src, const char *dst, int use_bwt, int use_mtf, int use_rle)
{
    FILE *fp = fopen(src, "rb");
    if (!fp)
    {
        perror("fopen");
        return -1;
    }

    // tags - check if the file was actually compressed by us, and it has the pipeline spec
    char tag[5] = {0};
    // read em and weap
    if (fread(tag, 1, 4, fp) != 4 || strcmp(tag, START) != 0)
    {
        fprintf(stderr, "bad file\n");
        fclose(fp);
        return -1;
    }
    // extracting all of the data header
    uint8_t flags = fgetc(fp);
    uint64_t orig_len = le64_read(fp); // og length
    uint64_t primary = le64_read(fp);  // pimary for bwt
    uint32_t bit_len = le32_read(fp);  // number of bits in huffman stream

    // reading the array of huffman code lengths
    uint8_t huff_length[HUFFMAN_SIZE];
    // fread em and weap
    if (fread(huff_length, 1, HUFFMAN_SIZE, fp) != HUFFMAN_SIZE)
    {
        fprintf(stderr, "truncated header\n");
        fclose(fp);
        return -1;
    }
    // data structure for building the cannonical codes
    uint32_t huff_codes[HUFFMAN_SIZE] = {0};
    uint32_t bit_len_count[HUFFMAN_SIZE + 1] = {0}, next_code[HUFFMAN_SIZE + 1];
    for (int s = 0; s < HUFFMAN_SIZE; ++s) {
         bit_len_count[huff_length[s]]++;
    }
    // setting up the decoding for the cannons -- this should probs be in huffman.c but anywho
    bit_len_count[0] = 0;
    uint32_t code = 0;
    next_code[0] = 0;
    // building up the codes 
    for (int bits = 1; bits <= HUFFMAN_SIZE; ++bits)
    {
        code = (code + bit_len_count[bits - 1]) << 1;
        next_code[bits] = code;
    }
    // getting the lengths for each of the symbols
    for (int s = 0; s < HUFFMAN_SIZE; ++s) {
        if (huff_length[s]) {
            huff_codes[s] = next_code[huff_length[s]]++;
        }
    }
        
    // since creating the buffer for the bytes of symbols 
    size_t byte_len = (bit_len + 7) / 8;
    unsigned char *bitbuf = malloc(byte_len);
    if (!bitbuf)
    {
        perror("malloc");
        fclose(fp);
        return -1;
    }
    // read in
    if (fread(bitbuf, 1, byte_len, fp) != byte_len)
    {
        fprintf(stderr, "truncated stream\n");
        fclose(fp);
        free(bitbuf);
        return -1;
    }
    fclose(fp);
    // now we have the codes, bitbuffer and the bit lengths so we need to find the final length 
    size_t len = huff_decode(bitbuf, bit_len, huff_length, huff_codes, NULL);
    // now that we have the actual length we allocate the space 
    unsigned char *buf = malloc(len);
    // and we do the actual decode -- probs could allocate the needed space so we only have to do this once but -- anywho
    huff_decode(bitbuf, bit_len, huff_length, huff_codes, buf);
    free(bitbuf);
    
    // run length
    if (flags & FLAG_RLE)
    {
        size_t out_len;
        unsigned char *tmp = (unsigned char *)rle_decode((char *)buf, len, &out_len);
        free(buf);
        buf = tmp;
        len = out_len;
    }

    // move to front 
    if (flags & FLAG_MTF)
    {
        size_t out_len;
        unsigned char *tmp = mtf_decode(buf, len, &out_len);
        free(buf);
        buf = tmp;
        len = out_len;
    }

    // big boy burrows 
    if (flags & FLAG_BWT)
    {
        unsigned char *tmp = bwt_decode(buf, len, primary);
        free(buf);
        buf = tmp;
    }

    FILE *out = fopen(dst, "wb");
    if (!out)
    {
        perror("out");
        free(buf);
        return -1;
    }
    fwrite(buf, 1, orig_len, out);
    fclose(out);
    free(buf);
    return 0;
}


// helpers for writing litten endian
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
    for (int i = 0; i < 8; i++)
    {
        int c = fgetc(fp);
        if (c == EOF)
        {
            perror("file");
            exit(1);
        }
        v |= ((uint64_t)c) << (i * 8);
    }
    return v;
}

static uint32_t le32_read(FILE *fp)
{
    uint32_t v = 0;
    for (int i = 0; i < 4; i++)
    {
        int c = fgetc(fp);
        if (c == EOF)
        {
            perror("file");
            exit(1);
        }
        v |= ((uint32_t)c) << (i * 8);
    }
    return v;
}

static void replace_buffer(unsigned char **buf, unsigned char *new_buf, unsigned char *original)
{
    if (*buf != original)
        free(*buf);
    *buf = new_buf;
}

static uint8_t get_max_len(const uint8_t *hlen)
{
    uint8_t max = 0;
    for (int i = 0; i < HUFFMAN_SIZE; ++i)
        if (hlen[i] > max)
            max = hlen[i];
    return max;
}