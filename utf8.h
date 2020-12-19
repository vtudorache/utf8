#ifndef __VT_UTF8_H__
#define __VT_UTF8_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>

/*
`utf8_get_rune` gets the next rune in the readable stream `input`.
Returns the rune.
Returns 0xfffd if the first characters in stream don't form a valid UTF-8 
sequence or another error occured.  
Returns (size_t)-1 if the end-of-file has been reached.
The variable `errno` is set to EILSEQ if an invalid or incomplete sequence 
was found, or to the last error code set by the standard library function 
`fgetc`.
*/
int32_t utf8_get_rune(FILE *input);

/*
`utf8_put_rune` puts in the writable stream `output` the UTF-8 bytes 
encoding `rune`.
Returns the value of `rune` in the absence of error.
Returns (size_t)-1 if the operation fails. The `errno` variable is set to
EILSEQ if `rune` isn't a valid code point or to the last error code set by
the standard library function `fputc`.
*/
int32_t utf8_put_rune(int32_t rune, FILE *output);

/*
`utf8_decode` writes at the address given by `rune` the code point obtained 
from parsing at most `n_bytes` characters of the zero-ending string `s`.
Returns the number of characters parsed.
Returns 0 if the first characters within `n_bytes` don't form a valid UTF-8 
sequence or the resulting code point is invalid.
The source pointer `s` can't be NULL.
*/
size_t utf8_decode(int32_t *rune, const char *s, size_t n_bytes);

/*
`utf8_encode` writes at the address given by `p` the UTF-8 sequence encoding
`rune`.
Returns the number of bytes written, even if `p` is NULL.
Returns 0 if `rune` is not a valid code point (the surrogate range is not
valid).
*/
size_t utf8_encode(char *p, int32_t rune);

/*
`utf8_to_wchars` writes at the address given by `buffer` (when not NULL) up 
to `count` wide characters converted from the valid UTF-8 characters of the 
zero-ending string `s`. Partial sequences are not converted.
Returns the number of non-zero wide characters written (even if `buffer` 
is NULL).
Returns 0 if the string `s` is empty ("\0").
Returns 0 and sets the global variable `errno` to EINVAL if `s` is NULL.
Returns (size_t)-1 if `s`contains invalid UTF-8 sequences.
*/
size_t utf8_to_wchars(wchar_t *buffer, const char *s, size_t count);

/*
`utf8_of_wchars` writes at the address given by `buffer` (when not NULL) up 
to `count` characters converted from the wide characters of the zero-ending 
wide string `p`. Partial sequences are not converted.
Returns the number of non-zero bytes written (even if `buffer` 
is NULL).
Returns 0 if the string `p` is empty ("\0").
Returns 0 and sets the global variable `errno` to EINVAL if `p` is NULL.
Returns (size_t)-1 if `p` can't convert to valid UTF-8.
*/
size_t utf8_of_wchars(char *buffer, const wchar_t *p, size_t count);

/*
`utf8_to_local` writes at the address given by `buffer` (when not NULL) up to 
`count` locale encoded characters converted from the UTF-8 characters of the 
zero-ending string `s`. Partial sequences are not converted.
Returns the number of non-zero bytes written (even if `buffer` 
is NULL).
Returns 0 if the string `s` is empty ("\0").
Returns 0 and sets the global variable `errno` to EINVAL if `s` is NULL.
Returns (size_t)-1 if `s` can't convert to valid UTF-8.
*/
size_t utf8_to_local(char *buffer, const char *s, size_t count);

/*
`utf8_of_local` writes at the address given by `buffer` (when not NULL) up to 
`count` characters converted from the locale encoded characters of the 
zero-terminated string `s`. Partial sequences are not converted.
Returns the number of non-zero bytes written (even if `buffer` is NULL).
Returns 0 if the string `s` is empty ("\0").
Returns 0 and sets the global variable `errno` to EINVAL if `s` is NULL.
Returns (size_t)-1 if `s` can't convert to valid UTF-8.
*/
size_t utf8_of_local(char *buffer, const char *s, size_t count);

/*
`utf8_of_ascii` writes at the address given by `buffer` (when not NULL) up 
to `count` UTF-8 bytes converted from the ASCII characters of the 
zero-terminated string `s`.
Translates sequences like `\xDD`, `\uDDDD` and `\UDDDDDDDD` to the UTF-8
equivalent of the rune with the given value, `D` in the examples above
meaning a hexadecimal digit. The sequence `\\` is converted to a single
backslash, so `\\uDDDD` is interpreted as '\', 'u', 'D', 'D', 'D', 'D' and
not translated to UTF-8. A backslash followed by any other character is
written as it is to the output buffer.
Partial sequences are not converted.
Returns the number of non-zero bytes written (even if `buffer` 
is NULL).
Returns 0 if the string `s` is empty ("\0").
Returns 0 and sets the global variable `errno` to EINVAL if `s` is NULL.
Returns (size_t)-1 if `s` can't convert to valid UTF-8 or if there are 
non-ASCII characters in the input (> 127).
*/
size_t utf8_of_ascii(char *buffer, char *s, size_t count);

#endif
