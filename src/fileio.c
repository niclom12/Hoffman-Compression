/**
 * This is the code used to read in the files and break them up into their tokens/symbles. Plan to extend to other file types
 */
#include "fileio.h"
#include "minheap.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

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

// Adapted from this https://www.geeksforgeeks.org/run-length-encoding/
char *runLengthEncode(char *buffer, long length)
{
    // Written for how many bytes written
    int runLength, written, j = 0;
    // Max size (unfortunately) (If every char is a number)
    int maxSize = (length * 2 + 1);
    char *encoded = (char *)malloc(sizeof(char) * maxSize);

    for (size_t i = 0; i < length; i++)
    {
        // Escape digits and escape escapes
        if (isdigit(buffer[i])) {
            encoded[j++] = '\\';
        } else if (buffer[i] == '\\') {
            encoded[j++] = '\\';
            encoded[j++] = '\\';
        }
        // Copy first occurence
        encoded[j++] = buffer[i];

        runLength = 1;
        while (i + 1 < length && buffer[i] == buffer[i + 1])
        {
            runLength++;
            i++;
        }

        // A -> A, not A -> A1
        if (runLength == 1) {
            continue;
        }

        // Slightly more effecient than geeksforgeeks method. 
        written = snprintf(encoded + j, maxSize - j, "%d", runLength);

        if (written < 0 || written >= maxSize - j)
        {
            // Panic -> This should never happen.
        }
        else
        {
            j += written;
        }
    }
    encoded[j] = '\0';
    return encoded;
}

// Adapted from this https://www.geeksforgeeks.org/run-length-encoding/
char *runLengthDecode(char *buffer, long length)
{
    // We assume decoded is at least double and double size each time its bigger
    // ? We could run through the buffer first to figure out size but I thought this was slightly faster (albeit not space effecient).
    int decodedCurLength = (length + 1) * 2, repeats, j = 0, i = 0;
    char *decoded = (char *) malloc(sizeof(char) * decodedCurLength);
    char currentChar;
    
    while (i < length)
    {
        // Get next character, and escapes ends previous runs
        currentChar = buffer[i++];
        if (currentChar == '\\') {
            currentChar = buffer[i++];
        }

        // Not garunteed to have a digit next (because could have just A of example).
        repeats = 0;
        while (isdigit(buffer[i]))
        {
            repeats = repeats * 10 + (buffer[i++] - '0');
        }

        // Resize (doubling strategy)
        if (j + repeats >= decodedCurLength) {
            decodedCurLength *= 2;
            char *tmp = (char *) malloc(sizeof(char) * decodedCurLength);
            memcpy(tmp, decoded, j);
            free(decoded);
            decoded = tmp;
        }        

        // A4 -> AAAA, but also A -> A (we do not expect A1)
        if (repeats == 0) {
            repeats = 1;
        }
        for (size_t k = 0; k < repeats; k++)
        {
            decoded[j++] = currentChar;
        }
    }
    decoded[j] = '\0';

    return decoded;
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

    // Call unit tests of min heap
    unit_test();

    // Automatically freed
    char *buffer = "AAAaaaBB   BCCCDDDEEEEEeEE!!?!!__-- XYZxXxXxX112315982039142333333333333333333\\"
                   "LLLLLLLLLlllllllllZZZZZzzzzzMMMMMnnnpppppPpPpP";
    printf("Original version: %s\n", buffer);
    char *buffer2 = runLengthEncode(buffer, strlen(buffer));
    printf(" Encoded version: %s\n", buffer2);
    char *buffer3 = runLengthDecode(buffer2, strlen(buffer2));
    printf(" Decoded version: %s\n", buffer3);
    // Memory management
    free(buffer2);
    free(buffer3);

    return 0;
}