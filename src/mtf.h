// move to front logical abstractions
#ifndef MTF_H
#define MTF_H

// This performs the encoding and retuns the integer array
int* mtf_encode(const char* input, int* out_len);

// Decodes the interger array into original character string
char* mtf_decode(const int* input, int len);

#endif
