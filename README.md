# utf8

## Overview
This is a small library for UTF-8 translations written in C. I've started 
this work because I've always found annoying to use directly the standard 
C library functions `mbstowcs`, `wcstombs` and the like to convert a locally 
encoded string to and from UTF-8, especially on Windows.  
The conversion functions act in the same way as the standard C library 
traditional functions: passing a NULL pointer for the destination buffer makes the 
function return the number of destination units (`char` or `wchar_t`) needed
for the destination buffer, *excluding* the final 0. If the destination 
pointer is not NULL, the terminator is added if and only if it is present 
in the source. But a 0 string terminator can easily be added manually as 
shown in the examples below.  
The functions `utf8_to_wchars` and `utf8_of_wchars` convert to and from the
platform-specific wide characters (that is, UTF-32 on Unix and UTF-16 on 
Windows).  
The functions `utf8_to_locale` and `utf8_of_locale` convert to and
from the local character set (code page). If the current locale uses
UTF-8 these functions are not needed. They work however on Unix (and Linux)
allowing the use of UTF-8 in any application even when the local code page is
ISO/ANSI.

## Index
[`size_t utf8_decode(int32_t *rune, const char *s, size_t n_bytes)`](https://github.com/vtudorache/utf8#utf8_decode)  
[`size_t utf8_encode(char *p, int32_t rune)`](https://github.com/vtudorache/utf8#utf8_encode)  
[`size_t utf8_to_wchars(wchar_t *buffer, const char *s, size_t count)`](https://github.com/vtudorache/utf8#utf8_to_wchars)  
[`size_t utf8_of_wchars(char *buffer, const wchar_t *p, size_t count)`](https://github.com/vtudorache/utf8#utf8_of_wchars)  
[`size_t utf8_to_locale(char *buffer, const char *s, size_t count)`](https://github.com/vtudorache/utf8#utf8_to_locale)  
[`size_t utf8_of_locale(char *buffer, const char *s, size_t count)`](https://github.com/vtudorache/utf8#utf8_of_locale)  

## Examples
[`size_t utf8_decode(int32_t *rune, const char *s, size_t n_bytes)`](https://github.com/vtudorache/utf8#example-utf8_decode)  
[`size_t utf8_encode(char *p, int32_t rune)`](https://github.com/vtudorache/utf8#example-utf8_encode)  
[`size_t utf8_to_wchars(wchar_t *buffer, const char *s, size_t count)`](https://github.com/vtudorache/utf8#example-utf8_to_wchars)  
[`size_t utf8_of_wchars(char *buffer, const wchar_t *p, size_t count)`](https://github.com/vtudorache/utf8#example-utf8_of_wchars)  
[`size_t utf8_to_locale(char *buffer, const char *s, size_t count)`](https://github.com/vtudorache/utf8#example-utf8_to_locale)  
[`size_t utf8_of_locale(char *buffer, const char *s, size_t count)`](https://github.com/vtudorache/utf8#example-utf8_of_locale)  

## Source
[utf8.c](https://github.com/vtudorache/utf8/blob/main/utf8.c)  

### **utf8_decode**
`size_t utf8_decode(int32_t *rune, const char *s, size_t n_bytes)`

Writes at the address given by `rune` the code point obtained from parsing
at most `n_bytes` characters of the zero-terminated string `s`.  
Returns the number of characters parsed, even when `rune` is NULL.  
Returns 0 if the first characters within `n_bytes` don't form a valid UTF-8 
sequence or the resulting code point is invalid.  
The source pointer `s` can't be NULL.  

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
Returns the number of characters used, even when `p` is NULL.  
Returns 0 if `rune` is not a valid code point (the surrogate range is not
valid).  

#### **Example (utf8_encode)**
```
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
```

### **utf8_to_wchars**
`size_t utf8_to_wchars(wchar_t *buffer, const char *s, size_t count)`

Writes at the address given by `buffer` (when not NULL) up to `count` wide 
characters converted from the valid UTF-8 characters of the zero-terminated 
string `s`. Partial sequences are not converted.  
Returns the number of non-zero wide characters converted (even if `buffer` 
is NULL).  
Returns 0 if the string `s` is empty (`"\0"`).  
Returns 0 and sets the global variable `errno` to EINVAL if `s` is NULL.  
Returns `(size_t)-1` if `s`contains invalid UTF-8 sequences.

#### **Example (utf8_to_wchars)**
```/* work in progress */```

### **utf8_of_wchars**
`size_t utf8_of_wchars(char *buffer, const wchar_t *p, size_t count)`

Writes at the address given by `buffer` (when not NULL) up to `count` 
characters converted from the wide characters of the zero-terminated wide
string `p`. Partial sequences are not converted.  
Returns the number of non-zero bytes converted (even if `buffer` 
is NULL).  
Returns 0 if the string `p` is empty (`"\0"`).  
Returns 0 and sets the global variable `errno` to EINVAL if `p` is NULL.  
Returns `(size_t)-1` if `p` can't convert to valid UTF-8.

#### **Example (utf8_of_wchars)**
```/* work in progress */```

### **utf8_to_locale**
`size_t utf8_to_locale(char *buffer, const char *s, size_t count)`

Writes at the address given by `buffer` (when not NULL) up to `count` 
locale encoded characters converted from the UTF-8 characters of the 
zero-terminated string `s`. Partial sequences are not converted.  
Returns the number of non-zero bytes converted (even if `buffer` 
is NULL).  
Returns 0 if the string `s` is empty (`"\0"`).  
Returns 0 and sets the global variable `errno` to EINVAL if `s` is NULL.  
Returns `(size_t)-1` if `s` can't convert to valid UTF-8.

#### **Example (utf8_to_locale)**
See [`size_t utf8_of_locale(char *buffer, const char *s, size_t count)`](https://github.com/vtudorache/utf8#example-utf8_of_locale).

### **utf8_of_locale**
`size_t utf8_of_locale(char *buffer, const char *s, size_t count)`

Writes at the address given by `buffer` (when not NULL) up to `count` 
characters converted from the locale encoded characters of the 
zero-terminated string `s`. Partial sequences are not converted.  
Returns the number of non-zero bytes converted (even if `buffer` 
is NULL).  
Returns 0 if the string `s` is empty (`"\0"`).  
Returns 0 and sets the global variable `errno` to EINVAL if `s` is NULL.  
Returns `(size_t)-1` if `s` can't convert to valid UTF-8.

#### **Example (utf8_of_locale)**
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
            printf("The string \"%s\" isn't valid UTF-8.\n", argv[i]);
            printf("Trying to convert from locale...\n");
            n = utf8_of_locale(NULL, argv[i], (size_t)-1);
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
            n = utf8_of_locale(s, argv[i], n + 1);
            n = utf8_to_wchars(NULL, s, (size_t)-1);
        } else {
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
            n = utf8_to_locale(NULL, s, (size_t)-1);
            if (n > 0) {
                r = (char *)malloc(n + 1);
                if (r != NULL) {
                    n = utf8_to_locale(r, s, n + 1);
                    printf("The string reconverted to locale is \"%s\".\n",
                        r);
                    free(r);
                }
            }
            free(s);
        }
    }
    return argc - i;
}
```

