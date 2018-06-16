#include <stdio.h>
#include <stdlib.h>

#include "config.h"

static const unsigned char lookup[] = {
    ['0'] = 0x00, ['1'] = 0x01, ['2'] = 0x02, ['3'] = 0x03,
    ['4'] = 0x04, ['5'] = 0x05, ['6'] = 0x06, ['7'] = 0x07,
    ['8'] = 0x08, ['9'] = 0x09, ['A'] = 0x0A, ['B'] = 0x0B,
    ['C'] = 0x0C, ['D'] = 0x0D, ['E'] = 0x0E, ['F'] = 0x0F,
};

int
main(void)
{
    char in[256];
    while (fgets(in, sizeof(in), stdin)) {
        unsigned char out[HASH_LENGTH];
        for (int i = 0; i < HASH_LENGTH; i++) {
            int nh = in[i * 2 + 0];
            int nl = in[i * 2 + 1];
            out[i] = (lookup[nh] << 4) | lookup[nl];
        }
        if (!fwrite(out, HASH_LENGTH, 1, stdout)) {
            perror(in);
            return EXIT_FAILURE;
        }
    }
    if (ferror(stdin)) {
        perror(in);
        return EXIT_FAILURE;
    }
}
