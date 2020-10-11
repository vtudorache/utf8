#include <stdio.h>
#include <stdint.h>

#include "utf8.h"

int main(int argc, char **argv)
{
    int i = 1;
    int32_t rune;
    size_t written;
    char s[4];
#if defined(_WIN32)
    puts("This example fully works only in UTF-8 enabled consoles.");
#endif
    while (i < argc) {
        rune = atoi(argv[i]);
        if (i == 1) puts("The UTF-8 string is:");
        if (rune > 0) {
            written = utf8_encode(s, rune);
            if (written == 0) {
                printf("The value 0x%0x isn't a valid rune.\n", rune);
                continue;
            }
            s[written] = 0;
            printf(s);
        }
        i++;
    }
    if (i < argc) 
        printf("There are %d arguments left.\n", argc - i);
    return argc - i;
}
