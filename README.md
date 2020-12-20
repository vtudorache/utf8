# utf8

## Overview

### This is work in progress.

**The library shows unexpected behaviour when compiled with Borland C++ free 
tools.** I'm currently testing the issue (`mbstowcs` and `wcstombs` functions
seem the culprits).

This is a small library for reading and converting to and from UTF-8 written 
in C. I've started this work because I've always found annoying to use directly 
the standard C library functions `mbstowcs`, `wcstombs` and the likes to convert 
a locally encoded string to UTF-8, especially on Windows.  
The conversion functions act in the same way as the standard C library 
functions: passing a `NULL` pointer for the destination buffer makes the 
function return the number of destination units (`char` or `wchar_t`) needed
for the destination buffer, *excluding* the final 0. If the destination 
pointer is not NULL, the terminator is added if and only if it is present 
in the source. But the `'\0'` string terminator can easily be added manually.  
The functions `utf8_to_wchars` and `utf8_of_wchars` convert to and from the
platform-specific wide characters (that is, UTF-32 on Unix and UTF-16 on 
Windows).  
The functions `utf8_to_local` and `utf8_of_local` convert to and
from the local character set (code page). If the current locale uses
UTF-8 these functions are not needed. They work however on Unix (and Linux)
allowing the use of UTF-8 in an application even when the local code page is
ISO/ANSI.

## Index
[`int32_t utf8_get_rune(FILE *input)`](#utf8_get_rune)  
[`int32_t utf8_put_rune(int32_t rune, FILE *output)`](#utf8_put_rune)

[`size_t utf8_decode(int32_t *rune, const char *s, size_t n_bytes)`](#utf8_decode)  
[`size_t utf8_encode(char *p, int32_t rune)`](#utf8_encode)

[`size_t utf8_to_wchars(wchar_t *buffer, const char *s, size_t count)`](#utf8_to_wchars)  
[`size_t utf8_of_wchars(char *buffer, const wchar_t *p, size_t count)`](#utf8_of_wchars)

[`size_t utf8_to_local(char *buffer, const char *s, size_t count)`](#utf8_to_local)  
[`size_t utf8_of_local(char *buffer, const char *s, size_t count)`](#utf8_of_local)  

[`size_t utf8_of_ascii(char *buffer, const char *s, size_t count)`](#utf8_of_ascii)  

## Examples
[`size_t utf8_decode(int32_t *rune, const char *s, size_t n_bytes)`](#example-utf8_decode)  
[`size_t utf8_encode(char *p, int32_t rune)`](#example-utf8_encode)

[`size_t utf8_of_local(char *buffer, const char *s, size_t count)`](#example-utf8_of_local)  

## Source
[`utf8.c`](https://github.com/vtudorache/utf8/blob/main/utf8.c)  

### **utf8_get_rune**  
`int32_t utf8_get_rune(FILE *input)`

Gets the next rune in the readable stream `input`.
Returns the rune.
Returns `0xfffd` if the first characters in stream don't form a valid UTF-8 
sequence or another error occured.  
Returns `-1` if the end-of-file has been reached.
The variable `errno` is set to `EILSEQ` if an invalid or incomplete sequence 
was found, or to the last error code set by the standard library function 
`fgetc`.

### **utf8_put_rune**  
`int32_t utf8_put_rune(int32_t rune, FILE *output)`

Puts to the writable stream `output` the UTF-8 bytes encoding `rune`.
Returns the value of `rune` in the absence of error.
Returns `-1` if the operation fails. The `errno` variable is set to
`EILSEQ` if `rune` isn't a valid code point or to the last error code set by
the standard library function `fputc`.

### **utf8_decode**
`size_t utf8_decode(int32_t *rune, const char *s, size_t n_bytes)`

Writes at the address given by `rune` the code point obtained from parsing
at most `n_bytes` characters of the zero-terminated string `s`.  
Returns the number of characters parsed, even when `rune` is `NULL`.  
Returns `0` if the first characters within `n_bytes` don't form a valid UTF-8 
sequence or the resulting code point is invalid.  
The source pointer `s` can't be `NULL`.  

#### **Example (utf8_decode)**
```
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
```

### **utf8_encode**
`size_t utf8_encode(char *p, int32_t rune)`

Writes at the address given by `p` the UTF-8 sequence encoding `rune`.  
Returns the number of characters written, even when `p` is `NULL`.  
Returns `0` if `rune` is not a valid code point (the surrogate range is
considered invalid).  

#### **Example (utf8_encode)**
```
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
        printf("Usage:\n%s <code_1> <code_2> ... <code_n>\n", argv[0]);
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
```

### **utf8_to_wchars**
`size_t utf8_to_wchars(wchar_t *buffer, const char *s, size_t count)`

Writes at the address given by `buffer` (when not `NULL`) up to `count` wide 
characters converted from the valid UTF-8 characters of the zero-terminated 
string `s`. Partial sequences are not converted and stop the conversion.  
Returns the number of non-zero wide characters written (even if `buffer` 
is `NULL`).  
Returns `0` if the string `s` is empty (`"\0"`).  
Returns `0` and sets the global variable `errno` to `EINVAL` if `s` is `NULL`.  
Returns `(size_t)-1` if `s`contains invalid UTF-8 sequences.

### **utf8_of_wchars**
`size_t utf8_of_wchars(char *buffer, const wchar_t *p, size_t count)`

Writes at the address given by `buffer` (when not `NULL`) up to `count` 
characters converted from the wide characters of the zero-terminated wide
string `p`. Partial sequences are not converted.  
Returns the number of non-zero bytes written (even if `buffer` 
is `NULL`).  
Returns `0` if the string `p` is empty (`"\0"`).  
Returns `0` and sets the global variable `errno` to `EINVAL` if `p` is `NULL`.  
Returns `(size_t)-1` if `p` can't convert to valid UTF-8.

### **utf8_to_local**
`size_t utf8_to_local(char *buffer, const char *s, size_t count)`

Writes at the address given by `buffer` (when not `NULL`) up to `count` 
locale encoded characters converted from the UTF-8 characters of the 
zero-terminated string `s`. Partial sequences are not converted.  
Returns the number of non-zero bytes written (even if `buffer` 
is `NULL`).  
Returns `0` if the string `s` is empty (`"\0"`).  
Returns `0` and sets the global variable `errno` to `EINVAL` if `s` is `NULL`.  
Returns `(size_t)-1` if `s` can't convert to valid UTF-8.

### **utf8_of_local**
`size_t utf8_of_local(char *buffer, const char *s, size_t count)`

Writes at the address given by `buffer` (when not `NULL`) up to `count` 
characters converted from the locale encoded characters of the 
zero-terminated string `s`. Partial sequences are not converted.  
Returns the number of non-zero bytes written (even if `buffer` 
is `NULL`).  
Returns `0` if the string `s` is empty (`"\0"`).  
Returns `0` and sets the global variable `errno` to `EINVAL` if `s` is `NULL`.  
Returns `(size_t)-1` if `s` can't convert to valid UTF-8.

#### **Example (utf8_of_local)**
```
#include <locale.h>
#include <stdio.h>

#include "utf8.h"

int main(int argc, char **argv)
{
    int i, j, s_is_alloc;
    wchar_t *pws;
    char *r, *s;
    size_t n;
    setlocale(LC_ALL, ""); /* this is always necessary */
    for (i = 1; i < argc; i++) {
        s_is_alloc = 0;
        printf("The argument #%d is \"%s\".\n", i, argv[i]);
        n = utf8_to_wchars(NULL, argv[i], (size_t)-1);
        if (n == 0) continue;
        if (n == (size_t)-1) {
            printf("The argument #%d isn't valid UTF-8.\n", i);
            printf("Trying to convert from locale...\n");
            n = utf8_of_local(NULL, argv[i], (size_t)-1);
            if (n == (size_t)-1) {
                printf("Can't convert \"%s\" to UTF-8.\n", argv[i]);
                continue;
            }
            s = (char *)malloc((n + 1) * sizeof(char));
            if (s == NULL) {
                printf("There is no more memory available.\n");
                return argc - i;
            }
            s_is_alloc = 1;
            n = utf8_of_local(s, argv[i], n + 1);
            n = utf8_to_wchars(NULL, s, (size_t)-1);
        } else {
            printf("The string \"%s\" is valid UTF-8.\n", argv[i]);
            s = argv[i];
        }
        printf("It can be converted to %zu non-zero wide characters.\n", n);
        pws = (wchar_t *)malloc((n + 1) * sizeof(wchar_t));
        if (pws == NULL) {
            printf("There is no more memory available.\n");
            return argc - i;
        }
        n = utf8_to_wchars(pws, s, n + 1);
        printf("The wide character codes are:\n");
        for (j = 0; j < n; j++) {
            if (j > 0) putchar(' ');
            printf("0x%x", pws[j]);
        }
        putchar('\n');
        free(pws);
        if (s_is_alloc) {
            n = utf8_to_local(NULL, s, (size_t)-1);
            if (n > 0) {
                r = (char *)malloc(n + 1);
                if (r != NULL) {
                    n = utf8_to_local(r, s, n + 1);
                    printf("The string reconverted to locale is \"%s\".\n", r);
                    free(r);
                }
            }
            free(s);
        }
    }
    return argc - i;
}
```
### **utf8_of_ascii**
`size_t utf8_of_ascii(char *buffer, const char *s, size_t count)`

Writes at the address given by `buffer` (when not `NULL`) up 
to `count` UTF-8 bytes converted from the ASCII characters of the 
zero-terminated string `s`.  
Translates sequences like `\xDD`, `\uDDDD` and `\UDDDDDDDD` to the UTF-8
equivalent of the rune with the given value, `D` in the examples above
meaning a hexadecimal digit. The sequence `\\` is converted to a single
backslash, so `\\uDDDD` is interpreted as '\', 'u', 'D', 'D', 'D', 'D' and
not translated to UTF-8. A backslash followed by any other character is
written as it is to the output buffer. Partial sequences are not converted.
Returns the number of non-zero bytes written (even if `buffer` 
is NULL).  
Returns 0 if the string `s` is empty (`"\0"`).  
Returns 0 and sets the global variable `errno` to `EINVAL` if `s` is `NULL`.  
Returns `(size_t)-1` if `s` can't convert to valid UTF-8 or if there are 
non-ASCII characters in the input (> 127).  
