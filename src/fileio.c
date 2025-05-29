/**
 * This is the code used to read in the files and break them up into their tokens/symbles. Plan to extend to other file types
 */
#include "fileio.h"
#include "minheap.h"
#include <stdlib.h>
#include "boolean.h"
#include "huffsize.h"

// Bit array for encoded / decoded version of buffer
Boolean *B;
// Index to where we are in encoded / decoded routine
int read_frequencies(const char *filename, int *freq) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return -1;
    memset(freq, 0, HUFFMAN_SIZE * sizeof(int));
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        freq[(unsigned char)ch]++;
    }
    fclose(fp);
    return 0;
}

char* read_text_file_to_buffer(const char *filename, long *length) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    *length = ftell(fp);
    rewind(fp);

    char *buffer = malloc((size_t)*length + 1);
    if (!buffer) { fclose(fp); return NULL; }

    size_t ret = fread(buffer, 1, (size_t)*length, fp);
    fclose(fp);
    if (ret < (size_t)*length) {
        free(buffer);
        return NULL;
    }
    buffer[*length] = '\0';
    return buffer;
}

uint8_t* read_text_file_to_buffer_u8(const char *filename, long *length) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    *length = ftell(fp);
    rewind(fp);

    uint8_t *buffer = malloc((size_t)*length + 1);
    if (!buffer) { fclose(fp); return NULL; }

    size_t ret = fread(buffer, 1, (size_t)*length, fp);
    fclose(fp);
    if (ret < (size_t)*length) {
        free(buffer);
        return NULL;
    }
    buffer[*length] = '\0';
    return buffer;
}


int write_binary_file(const char *filename, const uint8_t *data, long length) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) return -1;
    fwrite(data, 1, (size_t)length, fp);
    fclose(fp);
    return 0;
}

int write_text_file(const char *filename, const char *text, long length) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return -1;
    fwrite(text, 1, (size_t)length, fp);
    fclose(fp);
    return 0;
}

char *escape_whitespace(const uint8_t *in, size_t len, size_t *out_len) {
    // worst-case every byte doubles
    size_t cap = len * 2 + 1;
    char *out = malloc(cap);
    if (!out) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (j + 2 > cap) {
            cap *= 2;
            out = realloc(out, cap);
        }
        uint8_t c = in[i];
        if (c == ' ') {
            out[j++] = '\\'; out[j++] = 'w';
        } else if (c == '\t') {
            out[j++] = '\\'; out[j++] = 't';
        } else if (c == '\n') {
            out[j++] = '\\'; out[j++] = 'n';
        } else {
            out[j++] = (char)c;
        }
    }
    *out_len = j;
    return out;
}

// Reverse the process: "\w"→' ', "\t"→tab, "\n"→newline
uint8_t *unescape_whitespace(const char *in, size_t len, size_t *out_len) {
    // decoded can't be bigger than len
    uint8_t *out = malloc(len);
    if (!out) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (in[i] == '\\' && i + 1 < len) {
            char d = in[++i];
            if (d == 'w')       out[j++] = ' ';
            else if (d == 't')  out[j++] = '\t';
            else if (d == 'n')  out[j++] = '\n';
            else {
                // unknown escape—emit literally
                out[j++] = (uint8_t)'\\';
                out[j++] = (uint8_t)d;
            }
        } else {
            out[j++] = (uint8_t)in[i];
        }
    }
    *out_len = j;
    return out;
}