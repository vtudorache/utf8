#include <errno.h>
#include <stdio.h>
#include <stdint.h>

#include "utf8.h"

int main(int argc, char **argv)
{
    int i = 1;
    int32_t rune;
    size_t written;
    char *p, *s;
#if defined(_WIN32)
    puts("This example fully works only in UTF-8 enabled consoles.");
#endif
    if (argc > 1) {
        s = (char *)malloc((argc - 1) * 4 + 1);
        if (s == NULL) {
            fputs("ERROR: can't allocate the required size.\n", stderr);
            return -ENOMEM;
        }
        p = s;
    }
    while (i < argc) {
        rune = atoi(argv[i]);
        if (rune > 0) {
            written = utf8_encode(p, rune);
            if (written == 0) {
                fprintf(stderr, "The value 0x%0x isn't a valid rune.\n", 
                    rune);
                i++;
                continue;
            }
            p = &p[written];
        }
        i++;
    }
    *p = 0;
    printf("The UTF-8 string is:\n%s\n", s);
    if (i < argc) 
        printf("There are %d arguments left.\n", argc - i);
    return argc - i;
}
