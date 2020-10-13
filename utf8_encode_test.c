#include <errno.h>
#include <stdio.h>
#include <stdint.h>

#include "utf8.h"

static int32_t hex2rune(char *s)
{
    int32_t rune = 0;
    int d, i = 0; 
    while (s[i] != 0) {
        d = s[i];
        if (d == '0' && i == 0) {
            i++;
            continue;
        }
        if ((d == 'x' || d == 'X') && i == 1) {
            i++;
            continue;
        }
        if (d >= '0' && d <= '9') {
            d -= '0';
        } else if (d >= 'a' && d <= 'f') {
            d -= 'a';
            d += 10;
        } else if (d >= 'A' && d <= 'F') {
            d -= 'A';
            d += 10;
        } else {
            break;
        }
        rune = (rune << 4) | d;
        i++;
    }
    return rune;
}

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
    } else {
        printf("Usage:\n%s <hex_code_1> <hex_code_2> ... <hex_code_n>\n", argv[0]);
        return -EINVAL;
    }
    while (i < argc) {
        rune = hex2rune(argv[i]);
        if (rune > 0) {
            written = utf8_encode(p, rune);
            if (written == 0) {
                fprintf(stderr, "The value 0x%0x isn't a valid code point.\n", 
                    rune);
                i++;
                continue;
            }
            printf("Got code point 0x%x.\n", rune);
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
