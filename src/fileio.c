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