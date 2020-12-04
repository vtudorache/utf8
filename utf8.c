#include <errno.h>

#include "utf8.h"

static struct {
    int32_t lo;
    int32_t hi;
} utf8[] = {
    {0xd800, 0xdfff}, /* forbidden range: surrogates */
    {0, 0x7f},
    {0x80, 0x7ff},
    {0x800, 0xffff},
    {0x10000, 0x10ffff}
};

#if defined(_WIN32)

/* This is needed on Windows where wchar_t is 16-bit. */
static struct {
    int32_t lo;
    int32_t hi;
} utf16[] = {
    {0xd800, 0xdfff}, /* forbidden range: surrogates */
    {0, 0xffff},
    {0x10000, 0x10ffff}
};

#endif

/*
Writes at the address given by `rune` the code point obtained from parsing
at most `n_bytes` characters of the zero-terminated string `s`.
Returns the number of characters parsed.
Returns 0 if the first characters within `n_bytes` don't form a valid UTF-8 
sequence or the resulting code point is invalid.
The source pointer `s` can't be NULL.
*/
size_t utf8_decode(int32_t *rune, const char *s, size_t n_bytes)
{
    size_t n_cont = 0; /* number of continuation bytes */
    int32_t first, value = 0;
    if (n_bytes < 1 || (0xc0 & *s) == 0x80) return 0;
    if ((0x80 & *s) == 0) {
        if (rune != NULL) *rune = *s;
        return 1;
    }
    first = s[0];
    while (0x40 & first) {
        n_cont++;
        if (n_cont >= n_bytes) return 0; /* not enough bytes left */
        if (n_cont >= 4) return 0; /* sequence too long */
        if ((0xc0 & s[n_cont]) != 0x80) return 0; /* not a cont. byte */
        value = (value << 6) | (0x3f & s[n_cont]);
        first <<= 1;
    }
    value |= (0x7f & first) << (5 * n_cont);
    n_bytes = n_cont + 1; /* reuse n_bytes to hold the sequence size */
    if (utf8[n_bytes].hi < value || value < utf8[n_bytes].lo) return 0;
    if (rune != NULL) *rune = value;
    return n_bytes;
}

/*
Gets the next rune in the readable stream `input`.
Returns the rune.
Returns 0xfffd if the first characters in stream don't form a valid UTF-8 
sequence or another error occured.  
Returns (size_t)-1 if the end-of-file has been reached.
The variable `errno` is set to EILSEQ if an invalid or incomplete sequence 
was found, or to the last error code set by the standard library function 
`fgetc`.
*/
int32_t utf8_get_rune(FILE *input)
{
    size_t n_bytes, n_cont = 0; /* number of continuation bytes */
    int read;
    int32_t first, value = 0;
    if ((first = getc(input)) == EOF) return -1;
    if ((0xc0 & first) == 0x80) {
        errno = EILSEQ;
        return 0xfffd;
    }
    if ((0x80 & first) == 0) return first;
    while (0x40 & first) {
        read = getc(input);
        n_cont++;
        if (read == EOF || n_cont >= 4 || (0xc0 & read) != 0x80) {
            ungetc(read, input);
            errno = EILSEQ;
            return 0xfffd;
        }
        value = (value << 6) | (0x3f & read);
        first <<= 1;
    }
    value |= (0x7f & first) << (5 * n_cont);
    n_bytes = n_cont + 1;
    if (utf8[n_bytes].hi < value || value < utf8[n_bytes].lo) {
        errno = EILSEQ;
        return 0xfffd;
    }
    return value;
}

/*
Fills at most `buffer_size` bytes of `buffer` (including the final 0) with 
the UTF-8 sequences read from the stream `input`.
Replaces the invalid sequences found in `input` with `0xfffd`.
The write process stops when there's no more room left in `buffer`, when
an end-of-line or end-of-file is found.
Translates `\r\n` to `\n`.
Returns the number of bytes put in `buffer`, excluding the final '\0'.
*/
size_t utf8_get_bytes(char *buffer, size_t buffer_size, FILE *input)
{
    int32_t rune;
    size_t i = 0;
    if (buffer_size == 0 || input == NULL) return 0;
    if (buffer == NULL) {
        errno = EINVAL;
        return 0;
    }
    while (buffer_size > i + 4) {
        rune = utf8_get_rune(input);
        if (rune == -1) break;
        if (rune == '\r') {
            rune = getc(input);
            if (rune != '\n') {
                if (rune != EOF) ungetc(rune, input);
                rune = '\n';
            }
        }
        i += utf8_encode(&buffer[i], rune);
        if (rune == '\n') break;
    }
    buffer[i] = '\0';
    return i;
}

/*
Writes at the address given by `p` the UTF-8 sequence encoding `rune`.
Returns the number of characters used, even if `p` is NULL.
Returns 0 if `rune` is not a valid code point (the surrogate range is not
valid).
*/
size_t utf8_encode(char *p, int32_t rune)
{
    size_t i, n_bytes = 0; /* number of bytes to be written */
    while (n_bytes < sizeof(utf8)) {
        if (utf8[n_bytes].lo <= rune && rune <= utf8[n_bytes].hi) break;
        n_bytes++;
    }
    if (n_bytes < 1 || n_bytes >= sizeof(utf8)) return 0; /* invalid */
    if (p != NULL) {
        if (n_bytes == 1) {
            p[0] = rune;
        } else {
            for (i = n_bytes - 1; i > 0; i--) {
                p[i] = 0x80 | (0x3f & rune);
                rune >>= 6;
            }
            p[0] = (0xf00 >> n_bytes) | rune;
        }
    }
    return n_bytes;
}

/*
Puts in the writable stream `output` the UTF-8 bytes encoding `rune`.
Returns the value of `rune` in the absence of error.
Returns (size_t)-1 if the operation fails. The `errno` variable is set to
EILSEQ if `rune` isn't a valid code point or to the last error code set by
the standard library function `fputc`.
*/
int32_t utf8_put_rune(int32_t rune, FILE *output)
{
    size_t i, n_bytes = 0;
    int32_t done = rune;
    while (n_bytes < sizeof(utf8)) {
        if (utf8[n_bytes].lo <= rune && rune <= utf8[n_bytes].hi) break;
        n_bytes++;
    }
    if (n_bytes < 1 || n_bytes >= sizeof(utf8)) {
        errno = EILSEQ;
        return -1;
    }
    if (n_bytes == 1) {
        if (putc(rune, output) == EOF) return -1;
    } else {
        for (i = n_bytes - 1; i > 0; i--) {
            if (putc(0x80 | (0x3f & rune), output) == EOF) 
                return -1;
            rune >>= 6;
        }
        if (putc((0xf00 >> n_bytes) | rune, output) == EOF) 
            return -1;
    }
    return done;
}

/*
Writes to the stream `output` the UTF-8 sequences found in `buffer`, only
if `buffer` contains a valid UTF-8 string.
Returns 0 and sets `errno` to EINVAL if any argument is invalid.
Returns (size_t)-1 and sets `errno` to EILSEQ if `buffer` contains invalid 
sequences.
Returns (size_t)-1 and propagate the value of `errno` set by `fputs` when 
another error is found.
*/
size_t utf8_put_bytes(char *buffer, FILE *output)
{
    int32_t rune;
    size_t i = 0, j;
    if (buffer == NULL || output == NULL) {
        errno = EINVAL;
        return 0;
    }
    while (buffer[i] != '\0') {
        j = utf8_decode(&rune, &buffer[i], (size_t)-1);
        if (j < 1) {
            errno = EILSEQ;
            return (size_t)-1;
        }
        i += j;
    }
    if (fputs(buffer, output) == EOF) i = (size_t)-1;
    return i;
}

#if defined(_WIN32)

/*
Writes at the address given by `rune` the code point obtained from parsing
at most `n_wchars` UTF-16 wide characters of the zero-terminated string `p`.
Returns the number of UTF-16 wide characters parsed.
Returns 0 if the first wide characters within `n_wchars` don't form a valid 
UTF-16 sequence or the resulting code point is invalid.
The source pointer `p` can't be NULL.
*/
static size_t utf16_decode(int32_t *rune, const wchar_t *p, size_t n_wchars)
{
    int32_t value;
    if (n_wchars < 1) return 0; /* not enough wide characters left */
    if ((0xf800 & p[0]) != 0xd800) {
        if (rune != NULL) *rune = 0xffff & p[0]; /* clear the high part */
        return 1;
    }
    if ((0xfc00 & p[0]) == 0xd800) {
        if (n_wchars > 1 && (0xfc00 & p[1]) == 0xdc00) {
            value = 0x3ff & p[0];
            value = 0x10000 + ((value << 10) | (0x3ff & p[1]));
            if (rune != NULL) *rune = value;
            return 2;
        }
    }
    return 0;
}

/*
Writes at the address given by `p` the UTF-16 wide characters encoding `rune`.
Returns the number of UTF-16 wide characters used, even if `p` is NULL.
Returns 0 if `rune` is not a valid code point (the surrogate range is not
valid).
*/
static size_t utf16_encode(wchar_t *p, int32_t rune)
{
    size_t n_wchars = 0;
    while (n_wchars < sizeof(utf16)) {
        if (utf16[n_wchars].lo <= rune && rune <= utf16[n_wchars].hi) break;
        n_wchars++;
    }
    if (n_wchars < 1 || n_wchars >= sizeof(utf16)) return 0; /* invalid */
    if (p != NULL) {
        if (n_wchars == 1) {
            p[0] = rune;
        } else {
            rune -= 0x10000;
            p[1] = 0xdc00 | (0x3ff & rune);
            rune >>= 10;
            p[0] = 0xd800 | (0x3ff & rune);
        }
    }
    return n_wchars;
}

size_t utf8_to_wchars(wchar_t *buffer, const char *s, size_t count)
{
    int32_t rune;
    size_t done = 0, parsed, rune_size;
    if (s == NULL) {
        errno = EINVAL;
        return 0;
    }
    if (buffer == NULL) count = (size_t)-1;
    while (done < count) {
        parsed = utf8_decode(&rune, s, (size_t)-1);
        if (parsed == 0) {
            errno = EILSEQ;
            return (size_t)-1;
        }
        rune_size = utf16_encode(buffer, rune);
        if (rune_size > count - done || rune == 0) break;
        s = &s[parsed];
        if (buffer != NULL) buffer = &buffer[rune_size];
        done += rune_size;
    }
    return done;
}

size_t utf8_of_wchars(char *buffer, const wchar_t *p, size_t count)
{
    int32_t rune;
    size_t done = 0, parsed, rune_size;
    if (p == NULL) {
        errno = EINVAL;
        return 0;
    }
    if (buffer == NULL) count = (size_t)-1;
    while (done < count) {
        parsed = utf16_decode(&rune, p, (size_t)-1);
        if (parsed == 0) {
            errno = EILSEQ;
            return (size_t)-1;
        }
        rune_size = utf8_encode(buffer, rune);
        if (rune_size > count - done || rune == 0) break;
        p = &p[parsed];
        if (buffer != NULL) buffer = &buffer[rune_size];
        done += rune_size;
    }
    return done;
}

size_t utf8_to_locale(char *buffer, const char *s, size_t count)
{
    int32_t rune[2] = {0, 0};
    /*
    I use an array of two wide chars of which the last is always 0. This 
    allows using the wcstombs function knowing that its source will always 
    end in a 0 rune.
    */
    wchar_t ws_buffer[2] = {0, 0};
    size_t done = 0, parsed, mb_size;
    if (s == NULL) {
        errno = EINVAL;
        return 0;
    }
    if (buffer == NULL) count = (size_t)-1;
    while (done < count) {
        parsed = utf8_decode(rune, s, (size_t)-1);
        if (parsed == 0) {
            errno = EILSEQ;
            return (size_t)-1;
        }
        utf16_encode(ws_buffer, rune[0]); /* only the valid runes get here */
        mb_size = wcstombs(buffer, ws_buffer, (size_t)-1);
        if (mb_size == (size_t)-1) return (size_t)-1; /* can't encode */
        if (mb_size == 0 || mb_size > count - done) break;
        s = &s[parsed];
        if (buffer != NULL) buffer = &buffer[mb_size];
        done += mb_size;
    }
    return done;
}

size_t utf8_of_locale(char *buffer, const char *s, size_t count)
{
    int32_t rune;
    wchar_t ws_buffer[2] = {0, 0};
    size_t done = 0, mb_size, ws_size, rune_size;
    if (s == NULL) {
        errno = EINVAL;
        return 0;
    }
    if (buffer == NULL) count = (size_t)-1;
    while (done < count) {
        ws_size = mbstowcs(ws_buffer, s, 1);
        if (ws_size == (size_t)-1) return (size_t)-1;
        utf16_decode(&rune, ws_buffer, (size_t)-1);
        rune_size = utf8_encode(buffer, rune);
        if (rune_size > count - done || rune == 0) break;
        mb_size = wcstombs(NULL, ws_buffer, (size_t)-1);
        s = &s[mb_size];
        if (buffer != NULL) buffer = &buffer[rune_size];
        done += rune_size;
    }
    return done;
}

#else

size_t utf8_to_wchars(wchar_t *buffer, const char *s, size_t count)
{
    int32_t rune;
    size_t done = 0, parsed;
    if (s == NULL) {
        errno = EINVAL;
        return 0;
    }
    if (buffer == NULL) count = (size_t)-1;
    while (done < count) {
        parsed = utf8_decode(&rune, s, (size_t)-1);
        if (parsed == 0) {
            errno = EILSEQ;
            return (size_t)-1;
        }
        if (buffer != NULL) *buffer = (wchar_t)rune;
        if (rune == 0) break;
        s = &s[parsed];
        if (buffer != NULL) buffer = &buffer[1];
        done += 1;
    }
    return done;
}

size_t utf8_of_wchars(char *buffer, const wchar_t *p, size_t count)
{
    size_t done = 0, rune_size;
    if (p == NULL) {
        errno = EINVAL;
        return 0;
    }
    if (buffer == NULL) count = (size_t)-1;
    while (done < count) {
        rune_size = utf8_encode(buffer, (int32_t)*p);
        if (rune_size == 0) {
            errno = EILSEQ;
            return (size_t)-1;
        }
        if (rune_size > count - done || *p == 0) break;
        p = &p[1];
        if (buffer != NULL) buffer = &buffer[rune_size];
        done += rune_size;
    }
    return done;
}

size_t utf8_to_locale(char *buffer, const char *s, size_t count)
{
    int32_t rune[2] = {0, 0};
    size_t done = 0, parsed, mb_size;
    if (s == NULL) {
        errno = EINVAL;
        return 0;
    }
    if (buffer == NULL) count = (size_t)-1;
    while (done < count) {
        parsed = utf8_decode(rune, s, (size_t)-1);
        if (parsed == 0) {
            errno = EILSEQ;
            return (size_t)-1;
        }
        mb_size = wcstombs(buffer, rune, (size_t)-1);
        if (mb_size == (size_t)-1) return (size_t)-1;
        if (mb_size == 0 || mb_size > count - done) break;
        s = &s[parsed];
        if (buffer != NULL) buffer = &buffer[mb_size];
        done += mb_size;
    }
    return done;
}

size_t utf8_of_locale(char *buffer, const char *s, size_t count)
{
    wchar_t ws_buffer[2] = {0, 0};
    size_t done = 0, mb_size, ws_size, rune_size;
    if (s == NULL) {
        errno = EINVAL;
        return 0;
    }
    if (buffer == NULL) count = (size_t)-1;
    while (done < count) {
        ws_size = mbstowcs(ws_buffer, s, 1);
        if (ws_size == (size_t)-1) return (size_t)-1;
        rune_size = utf8_encode(buffer, (int32_t)*ws_buffer);
        if (rune_size > count - done || *ws_buffer == 0) break;
        mb_size = wcstombs(NULL, ws_buffer, (size_t)-1);
        s = &s[mb_size];
        if (buffer != NULL) buffer = &buffer[rune_size];
        done += rune_size;
    }
    return done;
}

#endif
