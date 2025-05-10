#ifndef FILEIO_H
#define FILEIO_H

#include <stdio.h>

#define MAX_SYMBOLS 256

// Frequency table from file
int read_frequencies(const char *filename, int freq[MAX_SYMBOLS]);

// Reads entire file into memory (optional)
char* read_text_file_to_buffer(const char *filename, long *length);

// Write binary compressed output
int write_binary_file(const char *filename, const unsigned char *data, long length);

// Write decompressed plain text
int write_text_file(const char *filename, const char *text, long length);

#endif