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

// Write binary compressed output
int write_binary_file(const char *filename, const uint8_t *data, long length);

// Write decompressed plain text
int write_text_file(const char *filename, const char *text, long length);

#endif