#ifndef FILEIO_H
#define FILEIO_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "minheap.h"
#include "boolean.h"

#define MAX_SYMBOLS 128 // Basic ASCII
#define HUFFMAN_SIZE (MAX_SYMBOLS * 2 - 1) // "Full" ASCII
#define DEBUG 1 // Controls if DEBUG messages should be shown in general.
#define DEBUG_EXTRA 0 // Not core DEBUG messages

// Frequency table from file
int read_frequencies(const char *filename, int *freq);

// Reads entire file into memory (optional)
char* read_text_file_to_buffer(const char *filename, long *length);

// Write binary compressed output
int write_binary_file(const char *filename, const __uint8_t *data, long length);

// Write decompressed plain text
int write_text_file(const char *filename, const char *text, long length);

// Builds huffman tree and returns the root. 
int buildHuffman(int *freq, int *left, int *right, int *parent);
// Encode entire buffer
void huffmanEncode(char *buffer, long k, int *left, int *parent, int root);
// Encode One Entry
void huffmanEncodeOne(int x, int *left, int *parent, int root);
// Decode entire buffer (assumes buildHuffman has run).
char *huffmanDecode(Boolean *B, int *left, int *right, int root, long length);

#endif