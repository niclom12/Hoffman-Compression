#ifndef SUFFIX_ARRAY_H
#define SUFFIX_ARRAY_H

#include <stddef.h>
#include <stdint.h>

int build_suffix_array(const uint8_t *txt, size_t n, int *sa);

#endif
