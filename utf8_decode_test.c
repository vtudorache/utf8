#include <stdio.h>
#include <stdint.h>

#include "utf8.h"

int main(int argc, char **argv)
{
    int i = 1;
    int32_t rune;
    size_t parsed;
    char *s;
#if defined(_WIN32)
    puts("This example fully works only in UTF-8 enabled consoles.");
#endif
    if (argc < 2) {
        printf("Usage:\n%s <string>\n", argv[0]);
        return -EINVAL;
    }
    while (i < argc) {
        s = argv[i];
        while (*s != 0) {
            parsed = utf8_decode(&rune, s, (size_t)-1);
            if (parsed == 0) {
                printf("The sequence \"%s\" isn't valid UTF-8.\n", s);
                break;
            }
            printf("Code point: 0x%0x\tsize: %zd byte(s)\n", 
                rune, parsed);
            s = &s[parsed];
        }
        i++;
    }
    if (i < argc) 
        printf("There are %d arguments left.\n", argc - i);
    return argc - i;
}
