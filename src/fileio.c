/**
 * This is the code used to read in the files and break them up into their tokens/symbles. Plan to extend to other file types
 */
#include "fileio.h"
#include <stdlib.h>

/**
 * Count character frequencies in a text file
 */
int read_frequencies(const char *filename, int freq[MAX_SYMBOLS]) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return -1;
    } 

    for (int i = 0; i < MAX_SYMBOLS; i++) {
        freq[i] = 0;
    }

    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        freq[(unsigned char)ch]++;
    }

    fclose(fp);
    return 0;
}

// Reads entire file into a buffer. Gonna use this for iteration when generating the hofman codes after the frequencies are calculated
char* read_text_file_to_buffer(const char *filename, long *length) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    *length = ftell(fp);
    rewind(fp);

    char *buffer = (char *)malloc(*length + 1);

    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    fread(buffer, 1, *length, fp);
    buffer[*length] = '\0';
    fclose(fp);
    return buffer;
}

// Write binary data to output file. For writing back compressed data
int write_binary_file(const char *filename, const unsigned char *data, long length) {
    FILE *fp = fopen(filename, "wb");

    if (!fp) {
        return -1;
    }

    fwrite(data, sizeof(unsigned char), length, fp);
    fclose(fp);
    return 0;
}

// Write plain text data (decompressed)
int write_text_file(const char *filename, const char *text, long length) {
    FILE *fp = fopen(filename, "w");

    if (!fp) {
        return -1;
    }

    fwrite(text, sizeof(char), length, fp);
    fclose(fp);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int freq[MAX_SYMBOLS];
    if (read_frequencies(argv[1], freq) != 0) {
        printf("Failed to open file: %s\n", argv[1]);
        return 1;
    }

    printf("Character Frequencies in '%s':\n", argv[1]);
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (freq[i] > 0) {
            if (i == '\n') {
                printf("'\\n' : %d\n", freq[i]);
            } else if (i == ' ') {
                printf("' '  : %d\n", freq[i]);
            } else {
                printf("'%c'  : %d\n", i, freq[i]);
            }
        }
    }

    return 0;
}