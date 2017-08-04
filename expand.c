#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
    unsigned char hash[20];
    char line[42] = {[40] = '\r', [41] = '\n'};

    while (fread(hash, sizeof(hash), 1, stdin)) {
        for (int i = 0; i < (int)sizeof(hash); i++) {
            line[i * 2 + 0] = "0123456789ABCDEF"[hash[i] >> 4];
            line[i * 2 + 1] = "0123456789ABCDEF"[hash[i] & 0xf];
        }
        if (!fwrite(line, sizeof(line), 1, stdout)) {
            perror(line);
            return EXIT_FAILURE;
        }
    }
    if (ferror(stdin)) {
        perror(line);
        return EXIT_FAILURE;
    }
}
