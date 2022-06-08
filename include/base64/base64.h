#ifndef B64_H
#define B64_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length);

unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length);

#endif // BASE64_H