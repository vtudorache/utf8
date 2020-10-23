#ifndef __VT_UTF8_H__
#define __VT_UTF8_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>

/*
utf8_get_rune
-------------------
Gets the next rune in the readable stream `input`.
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
utf8_put_rune
-------------------
Puts in the writable stream `output` the UTF-8 bytes encoding `rune`.
Returns the value of `rune` in the absence of error.
Returns (size_t)-1 if the operation fails. The `errno` variable is set to
EILSEQ if `rune` isn't a valid code point or to the last error code set by
the standard library function `fputc`.
*/
int32_t utf8_put_rune(int32_t rune, FILE *output);

/*
utf8_get_bytes
--------------
Fills at most `buffer_size` bytes of `buffer` (including the final 0) with 
the UTF-8 sequences read from the stream `input`.
Replaces the invalid sequences found in `input` with `0xfffd`.
The write process stops when there's no more room left in `buffer`, when
an end-of-line or end-of-file is found.
Returns the number of bytes put in `buffer`.
*/
size_t utf8_get_bytes(char *buffer, size_t buffer_size, FILE *input);

/*
utf8_put_bytes
--------------
Writes to the stream `output` the UTF-8 sequences found in `buffer`, only
if `buffer` contains a valid UTF-8 string.
Returns 0 and sets `errno` to EINVAL if `buffer` or `output` are NULL.
Returns (size_t)-1 and sets `errno` to EILSEQ if `buffer` contains invalid 
sequences.
Returns (size_t)-1 and propagate the value of `errno` set by `fputs` when 
another error is found.
*/
size_t utf8_put_bytes(char *buffer, FILE *output);

/*
utf8_decode
-----------
Writes at the address given by `rune` the code point obtained from parsing
at most `n_bytes` characters of the zero-terminated string `s`.
Returns the number of characters parsed.
Returns 0 if the first characters within `n_bytes` don't form a valid UTF-8 
sequence or the resulting code point is invalid.
The source pointer `s` can't be NULL.
*/
size_t utf8_decode(int32_t *rune, const char *s, size_t n_bytes);

/*
utf8_encode
-----------
Writes at the address given by `p` the UTF-8 sequence encoding `rune`.
Returns the number of characters used, even if `p` is NULL.
Returns 0 if `rune` is not a valid code point (the surrogate range is not
valid).
*/
size_t utf8_encode(char *p, int32_t rune);

/*
utf8_to_wchars
--------------
Writes at the address given by `buffer` (when not NULL) up to `count` wide 
characters converted from the valid UTF-8 characters of the zero-terminated 
string `s`. Partial sequences are not converted.
Returns the number of non-zero wide characters converted (even if `buffer` 
is NULL).
Returns 0 if the string `s` is empty ("\0").
Returns 0 and sets the global variable `errno` to EINVAL if `s` is NULL.
Returns (size_t)-1 if `s`contains invalid UTF-8 sequences.
*/
size_t utf8_to_wchars(wchar_t *buffer, const char *s, size_t count);

/*
utf8_of_wchars
--------------
Writes at the address given by `buffer` (when not NULL) up to `count` 
characters converted from the wide characters of the zero-terminated wide
string `p`. Partial sequences are not converted.
Returns the number of non-zero bytes converted (even if `buffer` 
is NULL).
Returns 0 if the string `p` is empty ("\0").
Returns 0 and sets the global variable `errno` to EINVAL if `p` is NULL.
Returns (size_t)-1 if `p` can't convert to valid UTF-8.
*/
size_t utf8_of_wchars(char *buffer, const wchar_t *p, size_t count);

/*
utf8_to_locale
--------------
Writes at the address given by `buffer` (when not NULL) up to `count` 
locale encoded characters converted from the UTF-8 characters of the 
zero-terminated string `s`. Partial sequences are not converted.
Returns the number of non-zero bytes converted (even if `buffer` 
is NULL).
Returns 0 if the string `s` is empty ("\0").
Returns 0 and sets the global variable `errno` to EINVAL if `s` is NULL.
Returns (size_t)-1 if `s` can't convert to valid UTF-8.
*/
size_t utf8_to_locale(char *buffer, const char *s, size_t count);

/*
utf8_of_locale
--------------
Writes at the address given by `buffer` (when not NULL) up to `count` 
characters converted from the locale encoded characters of the 
zero-terminated string `s`. Partial sequences are not converted.
Returns the number of non-zero bytes converted (even if `buffer` 
is NULL).
Returns 0 if the string `s` is empty ("\0").
Returns 0 and sets the global variable `errno` to EINVAL if `s` is NULL.
Returns (size_t)-1 if `s` can't convert to valid UTF-8.
*/
size_t utf8_of_locale(char *buffer, const char *s, size_t count);

#endif
