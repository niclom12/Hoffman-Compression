#ifndef SAIS_H
#define SAIS_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int build_suffix_array_sais(const uint8_t *T, size_t n, int *SA);

#endif // RLE_H