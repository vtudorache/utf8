#ifndef __VT_UTF8_H__
#define __VT_UTF8_H__

#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>

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
