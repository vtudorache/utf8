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
Writes at the address given by `rune` the code point obtained from parsing
at most `n_bytes` ASCII characters of the zero-terminated string `s`.
Returns the number of characters parsed.
Returns 0 if the first characters within `n_bytes` are't valid ASCII 
sequence or the resulting code point is invalid.
The source pointer `s` can't be NULL.
*/
static size_t ucs4_decode_ascii(int32_t *rune, const char *s, size_t n_bytes)
{
    size_t parsed = 0;
    int32_t r = 0;
    if (n_bytes < 1 || (0x80 & *s) != 0) return 0;
    if (*s != '\\') {
        if (rune != NULL) *rune = *s;
        return 1;
    }
    if (n_bytes < 2) return 0;
    parsed += 1;
    switch (s[parsed]) {
        case '\\':
            if (rune != NULL) *rune = '\\';
            return 2;
        case 'U':
            if (n_bytes < 10) return 0;
            n_bytes = 10;
            break;
        case 'u':
            if (n_bytes < 6) return 0;
            n_bytes = 6;
            break;
        case 'x':
            if (n_bytes < 4) return 0;
            n_bytes = 4;
            break;
        default:
            if (rune != NULL) *rune = '\\';	
            return 1;
    }
    parsed += 1;
    while (parsed < n_bytes) {
        if ('0' <= s[parsed] && s[parsed] <= '9')
            r = (r << 4) + (s[parsed] - '0');
        else if ('a' <= s[parsed] && s[parsed] <= 'f')
            r = (r << 4) + (s[parsed] - 'a' + 10);
        else if ('A' <= s[parsed] && s[parsed] <= 'F')
            r = (r << 4) + (s[parsed] - 'A' + 10);
        else
            return 0;
        parsed += 1;
    }
    if (r < 0 || r > 0x10ffff || (0xd800 <= r && r <= 0xdfff)) return 0;
    if (rune != NULL) *rune = r;
    return parsed;
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
    if ((first = fgetc(input)) == EOF) return -1;
    if ((0xc0 & first) == 0x80) {
        ungetc(first, input);
        errno = EILSEQ;
        return 0xfffd;
    }
    if ((0x80 & first) == 0) return first;
    while (0x40 & first) {
        read = fgetc(input);
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
        if (fputc(rune, output) == EOF) return -1;
    } else {
        for (i = n_bytes - 1; i > 0; i--) {
            if (fputc(0x80 | (0x3f & rune), output) == EOF) 
                return -1;
            rune >>= 6;
        }
        if (fputc((0xf00 >> n_bytes) | rune, output) == EOF) 
            return -1;
    }
    return done;
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
    /* don't write directly to `buffer`, encode in `cache` buffer first, */
    /* then check if the result fits within `count` output units         */
    wchar_t cache[2]; 
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
        rune_size = utf16_encode(cache, rune); /* encode to the cache */
        if (rune_size > count - done) break;
        if (buffer != NULL) { /* copy the cache */
            *buffer++ = cache[0];
            if (rune_size > 1) *buffer++ = cache[1];
        }
        if (rune == 0) break;
        s += parsed;
        done += rune_size;
    }
    return done;
}

size_t utf8_of_wchars(char *buffer, const wchar_t *p, size_t count)
{
    int32_t rune;
    size_t done = 0, parsed, rune_size;
    /* don't write directly to `buffer`, encode in `cache` buffer first, */
    /* then check if the result fits within `count` output units         */
    char cache[4];
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
        rune_size = utf8_encode(cache, rune);
        if (rune_size > count - done) break;
        if (buffer != NULL) {
            *buffer++ = cache[0];
            if (rune_size > 1) *buffer++ = cache[1];
            if (rune_size > 2) *buffer++ = cache[2];
            if (rune_size > 3) *buffer++ = cache[3];
        }
        if (rune == 0) break;
        p += parsed;
        done += rune_size;
    }
    return done;
}

size_t utf8_to_local(char *buffer, const char *s, size_t count)
{
    int32_t rune;
    /*
    I use an array of three wide chars of which the last is always 0. This 
    allows using the wcstombs function knowing that its source will always 
    end in a 0 rune.
    */
    wchar_t ws_buffer[] = {0, 0, 0};
    /* don't write directly to `buffer`, encode in `cache` buffer first, */
    /* then check if the result fits within `count` output units         */
    char cache[4];
    size_t done = 0, parsed, mb_size;
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
        ws_buffer[1] = 0; /* clear the (maybe left off) second wide character */
        utf16_encode(ws_buffer, rune); /* only the valid runes get here */
        mb_size = wcstombs(cache, ws_buffer, (size_t)-1);
        if (mb_size == (size_t)-1) return (size_t)-1; /* can't encode */
        if (mb_size > count - done) break;
        if (buffer != NULL) {
            *buffer++ = cache[0];
            if (mb_size > 1) *buffer++ = cache[1];
            if (mb_size > 2) *buffer++ = cache[2];
            if (mb_size > 3) *buffer++ = cache[3];
        }
        if (rune == 0) break;
        s += parsed;
        done += mb_size;
    }
    return done;
}

size_t utf8_of_local(char *buffer, const char *s, size_t count)
{
    int32_t rune;
    wchar_t ws_buffer[] = {0, 0, 0};
    /* don't write directly to `buffer`, encode in `cache` buffer first, */
    /* then check if the result fits within `count` output units         */
    char cache[4];
    size_t done = 0, mb_size, ws_size, rune_size;
    if (s == NULL) {
        errno = EINVAL;
        return 0;
    }
    if (buffer == NULL) count = (size_t)-1;
    while (done < count) {
        ws_buffer[1] = 0;
        ws_size = mbstowcs(ws_buffer, s, 1);
        if (ws_size == (size_t)-1) return (size_t)-1;
        if ((0xfc00 & ws_buffer[0]) == 0xd800) {
            /* surrogate pair */
            ws_size = mbstowcs(ws_buffer, s, 2);
            if (ws_size == (size_t)-1) return (size_t)-1;
        }
        utf16_decode(&rune, ws_buffer, (size_t)-1);
        rune_size = utf8_encode(cache, rune);
        if (rune_size > count - done) break;
        if (buffer != NULL) {
            *buffer++ = cache[0];
            if (rune_size > 1) *buffer++ = cache[1];
            if (rune_size > 2) *buffer++ = cache[2];
            if (rune_size > 3) *buffer++ = cache[3];
        }
        if (rune == 0) break;
        /* recalculate the size in the source string */
        mb_size = wcstombs(NULL, ws_buffer, (size_t)-1);
        s += mb_size;
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
        if (buffer != NULL) *buffer++ = (wchar_t)rune;
        if (rune == 0) break;
        s += parsed;
        done += 1;
    }
    return done;
}

size_t utf8_of_wchars(char *buffer, const wchar_t *p, size_t count)
{
    size_t done = 0, rune_size;
    /* don't write directly to `buffer`, encode in `cache` buffer first, */
    /* then check if the result fits within `count` output units         */
    char cache[4];
    if (p == NULL) {
        errno = EINVAL;
        return 0;
    }
    if (buffer == NULL) count = (size_t)-1;
    while (done < count) {
        rune_size = utf8_encode(cache, (int32_t)*p);
        if (rune_size == 0) {
            errno = EILSEQ;
            return (size_t)-1;
        }
        if (rune_size > count - done) break;
        if (buffer != NULL) {
            *buffer++ = cache[0];
            if (rune_size > 1) *buffer++ = cache[1];
            if (rune_size > 2) *buffer++ = cache[2];
            if (rune_size > 3) *buffer++ = cache[3];
        }
        if (*p == 0) break;
        p += 1;
        done += rune_size;
    }
    return done;
}

size_t utf8_to_local(char *buffer, const char *s, size_t count)
{
    int32_t rune[] = {0, 0};
    size_t done = 0, parsed, mb_size;
    /* don't write directly to `buffer`, encode in `cache` buffer first, */
    /* then check if the result fits within `count` output units         */
    char cache[4];
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
        mb_size = wcstombs(cache, rune, (size_t)-1);
        if (mb_size == (size_t)-1) return (size_t)-1;
        if (mb_size > count - done) break;
        if (buffer != NULL) {
            *buffer++ = cache[0];
            if (mb_size > 1) *buffer++ = cache[1];
            if (mb_size > 2) *buffer++ = cache[2];
            if (mb_size > 3) *buffer++ = cache[3];
        }
        if (*rune == 0) break;
        s += parsed;
        done += mb_size;
    }
    return done;
}

size_t utf8_of_local(char *buffer, const char *s, size_t count)
{
    wchar_t ws_buffer[] = {0, 0};
    size_t done = 0, mb_size, ws_size, rune_size;
    /* don't write directly to `buffer`, encode in `cache` buffer first, */
    /* then check if the result fits within `count` output units         */
    char cache[4];
    if (s == NULL) {
        errno = EINVAL;
        return 0;
    }
    if (buffer == NULL) count = (size_t)-1;
    while (done < count) {
        ws_size = mbstowcs(ws_buffer, s, 1);
        if (ws_size == (size_t)-1) return (size_t)-1;
        rune_size = utf8_encode(cache, (int32_t)*ws_buffer);
        if (rune_size > count - done) break;
        if (buffer != NULL) {
            *buffer++ = cache[0];
            if (rune_size > 1) *buffer++ = cache[1];
            if (rune_size > 2) *buffer++ = cache[2];
            if (rune_size > 3) *buffer++ = cache[3];
        }
        if (*ws_buffer == 0) break;
        mb_size = wcstombs(NULL, ws_buffer, (size_t)-1);
        s += mb_size;
        done += rune_size;
    }
    return done;
}

#endif

size_t utf8_of_ascii(char *buffer, const char *s, size_t count)
{
    int32_t rune;
    size_t done = 0, parsed, rune_size;
    char cache[4];
    if (s == NULL) {
        errno = EINVAL;
        return 0;
    }
    if (buffer == NULL) count = (size_t)-1;
    while (done < count) {
        parsed = ucs4_decode_ascii(&rune, s, (size_t)-1);
        if (parsed == 0) {
            errno = EILSEQ;
            return (size_t)-1;
        }
        rune_size = utf8_encode(cache, rune);
        if (rune_size > count - done) break;
        if (buffer != NULL) { /* copy the cache */
            *buffer++ = cache[0];
            if (rune_size > 1) *buffer++ = cache[1];
            if (rune_size > 2) *buffer++ = cache[2];
            if (rune_size > 3) *buffer++ = cache[3];
        }
        if (rune == 0) break;
        s += parsed;
        done += rune_size;
    }
    return done;
}
