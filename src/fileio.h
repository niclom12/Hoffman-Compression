#ifndef FILEIO_H
#define FILEIO_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "minheap.h"
#include "boolean.h"
#include "huffsize.h"
#include <stdlib.h>
#include <stdint.h>

#define MAX_SYMBOLS 128 // Basic ASCII
#define HUFFMAN_SIZE 128 // ASCII
#define DEBUG 1 // Controls if DEBUG messages should be shown in general.
#define DEBUG_EXTRA 0 // Not core DEBUG messages

// Frequency table from file
int read_frequencies(const char *filename, int *freq);

// Reads entire file into memory (optional)
char* read_text_file_to_buffer(const char *filename, long *length);

uint8_t *read_text_file_to_buffer_u8(const char *filename, long *length);

char  *escape_whitespace(const uint8_t *in, size_t len, size_t *out_len);
uint8_t *unescape_whitespace(const char  *in, size_t len, size_t *out_len);

// Write binary compressed output
int write_binary_file(const char *filename, const uint8_t *data, long length);

// Write decompressed plain text
int write_text_file(const char *filename, const char *text, long length);

#endif