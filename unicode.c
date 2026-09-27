/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 * 
 * It is available for the Linux and macOS operating systems.
 *
 */
/*
 * XENLY Unicode Support — unicode.c
 *
 * UTF-8 / UTF-16 codec, character classification, case folding,
 * grapheme clusters, display width, and normalization.
 *
 * Inspired by Go's unicode, unicode/utf8, and unicode/utf16 packages.
 * All string APIs operate on NUL-terminated UTF-8 byte sequences.
 * Codepoints are uint32_t (Go: rune / int32).
 */

#include "unicode.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* ═══════════════════════════════════════════════════════════════════════════
 *  INTERNAL HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Sequence-length table for a leading byte (0 = invalid, 1-4 = bytes) */
static const uint8_t utf8_seq_len[256] = {
    /* 0x00-0x7F  ASCII */
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    /* 0x80-0xBF  continuation bytes — invalid as leaders */
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    /* 0xC0-0xDF  2-byte leaders */
    0,0,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
    /* 0xE0-0xEF  3-byte leaders */
    3,3,3,3,3,3,3,3, 3,3,3,3,3,3,3,3,
    /* 0xF0-0xF7  4-byte leaders */
    4,4,4,4,4,0,0,0, 0,0,0,0,0,0,0,0
};

/* ═══════════════════════════════════════════════════════════════════════════
 *  UTF-8 CODEC
 * ═══════════════════════════════════════════════════════════════════════════ */

uint32_t utf8_decode(const char *str, size_t *bytes) {
    int valid;
    return utf8_decode_rune(str, bytes, &valid);
}

uint32_t utf8_decode_rune(const char *str, size_t *bytes, int *valid) {
    const unsigned char *s = (const unsigned char *)str;
    if (!s || !s[0]) { *bytes = 0; *valid = 0; return UNICODE_RUNE_ERROR; }

    uint8_t len = utf8_seq_len[s[0]];

    /* Invalid leading byte */
    if (len == 0) { *bytes = 1; *valid = 0; return UNICODE_RUNE_ERROR; }

    /* ASCII fast path */
    if (len == 1) { *bytes = 1; *valid = 1; return s[0]; }

    /* Validate continuation bytes exist */
    for (uint8_t i = 1; i < len; i++) {
        if ((s[i] & 0xC0) != 0x80) {
            *bytes = 1; *valid = 0; return UNICODE_RUNE_ERROR;
        }
    }

    uint32_t cp;
    switch (len) {
    case 2: cp = ((uint32_t)(s[0] & 0x1F) << 6)  | (s[1] & 0x3F); break;
    case 3: cp = ((uint32_t)(s[0] & 0x0F) << 12) | ((uint32_t)(s[1] & 0x3F) << 6)  | (s[2] & 0x3F); break;
    default:cp = ((uint32_t)(s[0] & 0x07) << 18) | ((uint32_t)(s[1] & 0x3F) << 12) |
                 ((uint32_t)(s[2] & 0x3F) << 6)  | (s[3] & 0x3F); break;
    }

    /* Overlong sequences and surrogates are invalid */
    if ((len == 2 && cp < 0x0080) ||
        (len == 3 && cp < 0x0800) ||
        (len == 4 && cp < 0x10000) ||
        (cp >= 0xD800 && cp <= 0xDFFF) ||
        cp > UNICODE_MAX_RUNE) {
        *bytes = 1; *valid = 0; return UNICODE_RUNE_ERROR;
    }

    *bytes = len; *valid = 1;
    return cp;
}

size_t utf8_encode(uint32_t cp, char *out) {
    if (cp < 0x80) {
        out[0] = (char)cp;
        return 1;
    } else if (cp < 0x800) {
        out[0] = (char)(0xC0 | (cp >> 6));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    } else if (cp < 0x10000) {
        if (cp >= 0xD800 && cp <= 0xDFFF) return 0; /* surrogates */
        out[0] = (char)(0xE0 | (cp >> 12));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    } else if (cp <= UNICODE_MAX_RUNE) {
        out[0] = (char)(0xF0 | (cp >> 18));
        out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[3] = (char)(0x80 | (cp & 0x3F));
        return 4;
    }
    return 0;
}

size_t utf8_strlen(const char *str) {
    if (!str) return 0;
    size_t len = 0, bytes;
    while (*str) { utf8_decode(str, &bytes); str += bytes; len++; }
    return len;
}

size_t utf8_strlen_n(const char *str, size_t n) {
    if (!str) return 0;
    size_t len = 0, bytes, consumed = 0;
    while (*str && consumed < n) {
        utf8_decode(str, &bytes);
        if (consumed + bytes > n) break;
        str += bytes; consumed += bytes; len++;
    }
    return len;
}

size_t utf8_byte_len(const char *str, size_t n) {
    if (!str) return 0;
    size_t total = 0, bytes;
    for (size_t i = 0; i < n && *str; i++) {
        utf8_decode(str, &bytes);
        str += bytes; total += bytes;
    }
    return total;
}

int utf8_is_valid(const char *str) {
    if (!str) return 1;
    size_t bytes; int valid;
    while (*str) {
        utf8_decode_rune(str, &bytes, &valid);
        if (!valid) return 0;
        str += bytes;
    }
    return 1;
}

int utf8_is_valid_n(const char *str, size_t n) {
    if (!str) return 1;
    size_t bytes, consumed = 0; int valid;
    while (*str && consumed < n) {
        utf8_decode_rune(str, &bytes, &valid);
        if (!valid) return 0;
        if (consumed + bytes > n) return 0;
        str += bytes; consumed += bytes;
    }
    return 1;
}

int utf8_is_continuation(unsigned char byte) {
    return (byte & 0xC0) == 0x80;
}

int utf8_full_rune(const char *str, size_t n) {
    if (!str || n == 0) return 0;
    unsigned char lead = (unsigned char)str[0];
    uint8_t need = utf8_seq_len[lead];
    if (need == 0) return 1; /* Invalid: treat as self-contained error */
    return n >= (size_t)need;
}

const char *utf8_next(const char *str) {
    if (!str || !*str) return str;
    size_t bytes;
    utf8_decode(str, &bytes);
    return str + bytes;
}

const char *utf8_prev(const char *str, const char *start) {
    if (!str || str == start) return str;
    str--;
    while (str > start && utf8_is_continuation((unsigned char)*str)) str--;
    return str;
}

const char *utf8_char_at(const char *str, size_t index) {
    if (!str) return str;
    size_t bytes;
    for (size_t i = 0; i < index && *str; i++) {
        utf8_decode(str, &bytes);
        str += bytes;
    }
    return str;
}

size_t utf8_index_byte(const char *str, size_t index) {
    if (!str) return 0;
    const char *start = str;
    size_t bytes;
    for (size_t i = 0; i < index && *str; i++) {
        utf8_decode(str, &bytes);
        str += bytes;
    }
    return (size_t)(str - start);
}

size_t utf8_slice(const char *src, size_t start, size_t end,
                  char *buf, size_t buf_size) {
    if (!src || !buf || buf_size == 0 || end <= start) {
        if (buf && buf_size) buf[0] = '\0';
        return 0;
    }
    src = utf8_char_at(src, start);
    size_t written = 0, bytes;
    for (size_t i = start; i < end && *src && written + 4 < buf_size; i++) {
        uint32_t cp = utf8_decode(src, &bytes);
        size_t enc = utf8_encode(cp, buf + written);
        written += enc;
        src += bytes;
    }
    buf[written] = '\0';
    return written;
}

int utf8_contains_rune(const char *str, uint32_t cp) {
    return utf8_index_rune(str, cp) != (size_t)-1;
}

size_t utf8_index_rune(const char *str, uint32_t cp) {
    if (!str) return (size_t)-1;
    const char *start = str;
    size_t bytes;
    while (*str) {
        uint32_t c = utf8_decode(str, &bytes);
        if (c == cp) return (size_t)(str - start);
        str += bytes;
    }
    return (size_t)-1;
}

size_t utf8_rune_count_n(const char *str, size_t n) {
    return utf8_strlen_n(str, n);
}

size_t utf8_count_rune(const char *str, uint32_t cp) {
    if (!str) return 0;
    size_t count = 0, bytes;
    while (*str) {
        uint32_t c = utf8_decode(str, &bytes);
        if (c == cp) count++;
        str += bytes;
    }
    return count;
}

size_t utf8_reverse(const char *src, char *buf, size_t buf_size) {
    if (!src || !buf || buf_size == 0) return 0;
    /* Collect runes */
    size_t len = utf8_strlen(src);
    if (len == 0) { buf[0] = '\0'; return 0; }
    uint32_t *runes = (uint32_t *)malloc(len * sizeof(uint32_t));
    if (!runes) { buf[0] = '\0'; return 0; }
    const char *p = src;
    size_t bytes;
    for (size_t i = 0; i < len; i++) {
        runes[i] = utf8_decode(p, &bytes);
        p += bytes;
    }
    /* Encode in reverse */
    size_t written = 0;
    for (size_t i = len; i > 0 && written + 4 < buf_size; i--) {
        written += utf8_encode(runes[i-1], buf + written);
    }
    buf[written] = '\0';
    free(runes);
    return written;
}

size_t utf8_sanitize(const char *src, char *buf, size_t buf_size) {
    if (!src || !buf || buf_size == 0) return 0;
    size_t written = 0, bytes; int valid;
    while (*src && written + 4 < buf_size) {
        uint32_t cp = utf8_decode_rune(src, &bytes, &valid);
        if (valid)
            written += utf8_encode(cp, buf + written);
        else
            written += utf8_encode(UNICODE_REPLACEMENT, buf + written);
        src += bytes;
    }
    buf[written] = '\0';
    return written;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  UTF-16 CODEC
 * ═══════════════════════════════════════════════════════════════════════════ */

int utf16_is_surrogate(uint32_t cp)      { return cp >= 0xD800 && cp <= 0xDFFF; }
int utf16_is_high_surrogate(uint32_t cp) { return cp >= 0xD800 && cp <= 0xDBFF; }
int utf16_is_low_surrogate(uint32_t cp)  { return cp >= 0xDC00 && cp <= 0xDFFF; }

int utf16_encode_pair(uint32_t cp, uint16_t *high, uint16_t *low) {
    if (cp < 0x10000 || cp > UNICODE_MAX_RUNE) return 0;
    cp -= 0x10000;
    *high = (uint16_t)(0xD800 | (cp >> 10));
    *low  = (uint16_t)(0xDC00 | (cp & 0x3FF));
    return 1;
}

uint32_t utf16_decode_pair(uint16_t high, uint16_t low) {
    if (!utf16_is_high_surrogate(high) || !utf16_is_low_surrogate(low))
        return UNICODE_RUNE_ERROR;
    return (uint32_t)0x10000 + (((uint32_t)(high - 0xD800)) << 10) + (low - 0xDC00);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  UNICODE CHARACTER PROPERTIES
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ── Letter / case ───────────────────────────────────────────────────────── */

int unicode_is_upper(uint32_t cp) {
    /* ASCII fast path */
    if (cp >= 'A' && cp <= 'Z') return 1;
    /* Latin-1 supplement uppercase */
    if (cp >= 0x00C0 && cp <= 0x00D6) return 1;
    if (cp >= 0x00D8 && cp <= 0x00DE) return 1;
    /* Greek uppercase U+0391-U+03A9 */
    if (cp >= 0x0391 && cp <= 0x03A9) return 1;
    /* Cyrillic uppercase U+0410-U+042F */
    if (cp >= 0x0410 && cp <= 0x042F) return 1;
    /* Additional Latin Extended */
    if (cp >= 0x0100 && cp <= 0x012E && (cp & 1) == 0) return 1;
    if (cp >= 0x0130 && cp <= 0x0136 && (cp & 1) == 0) return 1;
    if (cp >= 0x0139 && cp <= 0x0148 && (cp & 1) == 1) return 1;
    if (cp >= 0x014A && cp <= 0x017E && (cp & 1) == 0) return 1;
    /* Latin Extended-B and IPA */
    if (cp >= 0x0181 && cp <= 0x01F0) return 1; /* approximation */
    /* Letterlike, Number Forms uppercase */
    if (cp >= 0x2160 && cp <= 0x216F) return 1; /* Roman numerals uppercase */
    /* Enclosed Latin uppercase A-Z */
    if (cp >= 0x24B6 && cp <= 0x24CF) return 1;
    /* Halfwidth/fullwidth Latin uppercase */
    if (cp >= 0xFF21 && cp <= 0xFF3A) return 1;
    return 0;
}

int unicode_is_lower(uint32_t cp) {
    if (cp >= 'a' && cp <= 'z') return 1;
    if (cp >= 0x00DF && cp <= 0x00F6) return 1;
    if (cp >= 0x00F8 && cp <= 0x00FF) return 1;
    if (cp >= 0x03B1 && cp <= 0x03C9) return 1; /* Greek lowercase */
    if (cp >= 0x0430 && cp <= 0x044F) return 1; /* Cyrillic lowercase */
    if (cp >= 0x0100 && cp <= 0x012F && (cp & 1) == 1) return 1;
    if (cp >= 0x0131 && cp <= 0x0137 && (cp & 1) == 1) return 1;
    if (cp >= 0x013A && cp <= 0x0149 && (cp & 1) == 0) return 1;
    if (cp >= 0x014B && cp <= 0x017E && (cp & 1) == 1) return 1;
    if (cp >= 0x2170 && cp <= 0x217F) return 1; /* Roman numerals lowercase */
    if (cp >= 0x24D0 && cp <= 0x24E9) return 1; /* Enclosed Latin lowercase */
    if (cp >= 0xFF41 && cp <= 0xFF5A) return 1; /* Fullwidth Latin lowercase */
    return 0;
}

int unicode_is_title(uint32_t cp) {
    /* Titlecase letters — small set: DZ, Lj, Nj, Dz, etc. */
    static const uint32_t titles[] = {
        0x01C5, 0x01C8, 0x01CB, 0x01F2, /* Dz Lj Nj Dz */
        0x1F88, 0x1F89, 0x1F8A, 0x1F8B, 0x1F8C, 0x1F8D, 0x1F8E, 0x1F8F,
        0x1F98, 0x1F99, 0x1F9A, 0x1F9B, 0x1F9C, 0x1F9D, 0x1F9E, 0x1F9F,
        0x1FA8, 0x1FA9, 0x1FAA, 0x1FAB, 0x1FAC, 0x1FAD, 0x1FAE, 0x1FAF,
        0x1FBC, 0x1FCC, 0x1FFC, 0
    };
    for (int i = 0; titles[i]; i++)
        if (cp == titles[i]) return 1;
    return 0;
}

int unicode_is_letter(uint32_t cp) {
    if (unicode_is_upper(cp) || unicode_is_lower(cp) || unicode_is_title(cp))
        return 1;
    /* Modifier letters Lm */
    if (cp >= 0x02B0 && cp <= 0x02FF) return 1;
    if (cp >= 0x0300 && cp <= 0x036F) return 0; /* combining marks, not letters */
    /* Other letters Lo — broad ranges */
    if (cp >= 0x0041 && cp <= 0x007A) return 1; /* ASCII */
    if (cp >= 0x00AA && cp == 0x00AA) return 1;
    if (cp >= 0x00B5 && cp == 0x00B5) return 1;
    if (cp >= 0x00BA && cp == 0x00BA) return 1;
    if (cp >= 0x00C0 && cp <= 0x00FF) return 1;
    if (cp >= 0x0100 && cp <= 0x02C1) return 1;
    if (cp >= 0x0370 && cp <= 0x037D) return 1;
    if (cp >= 0x037F && cp <= 0x1FFF) return 1;
    if (cp >= 0x200C && cp <= 0x200D) return 1; /* ZWNJ ZWJ */
    if (cp >= 0x2070 && cp <= 0x218F) return 1;
    if (cp >= 0x2C00 && cp <= 0x2FEF) return 1;
    if (cp >= 0x3001 && cp <= 0xD7FF) return 1;
    if (cp >= 0xF900 && cp <= 0xFDCF) return 1;
    if (cp >= 0xFDF0 && cp <= 0xFFFD) return 1;
    if (cp >= 0x10000 && cp <= 0xEFFFF) return 1;
    return 0;
}

/* ── Digit / number ──────────────────────────────────────────────────────── */

int unicode_is_digit(uint32_t cp) {
    if (cp >= '0' && cp <= '9') return 1;
    /* Arabic-Indic digits */
    if (cp >= 0x0660 && cp <= 0x0669) return 1;
    if (cp >= 0x06F0 && cp <= 0x06F9) return 1;
    /* Devanagari */
    if (cp >= 0x0966 && cp <= 0x096F) return 1;
    /* Bengali, Gurmukhi, Gujarati, Oriya, Tamil, Telugu, Kannada, Malayalam */
    if (cp >= 0x09E6 && cp <= 0x09EF) return 1;
    if (cp >= 0x0A66 && cp <= 0x0A6F) return 1;
    if (cp >= 0x0AE6 && cp <= 0x0AEF) return 1;
    if (cp >= 0x0BE6 && cp <= 0x0BEF) return 1;
    if (cp >= 0x0CE6 && cp <= 0x0CEF) return 1;
    if (cp >= 0x0D66 && cp <= 0x0D6F) return 1;
    if (cp >= 0x0E50 && cp <= 0x0E59) return 1;  /* Thai digits */
    if (cp >= 0x0ED0 && cp <= 0x0ED9) return 1;  /* Lao digits */
    if (cp >= 0x0F20 && cp <= 0x0F29) return 1;  /* Tibetan digits */
    if (cp >= 0x1040 && cp <= 0x1049) return 1;  /* Myanmar digits */
    if (cp >= 0xFF10 && cp <= 0xFF19) return 1;  /* Fullwidth digits */
    return 0;
}

int unicode_digit_value(uint32_t cp) {
    if (cp >= '0' && cp <= '9') return (int)(cp - '0');
    if (cp >= 0x0660 && cp <= 0x0669) return (int)(cp - 0x0660);
    if (cp >= 0x06F0 && cp <= 0x06F9) return (int)(cp - 0x06F0);
    if (cp >= 0x0966 && cp <= 0x096F) return (int)(cp - 0x0966);
    if (cp >= 0xFF10 && cp <= 0xFF19) return (int)(cp - 0xFF10);
    if (!unicode_is_digit(cp)) return -1;
    /* For other scripts, find the base digit 0 */
    uint32_t base = cp & ~0xFU;
    return (int)(cp - base);
}

int unicode_is_number(uint32_t cp) {
    if (unicode_is_digit(cp)) return 1;
    /* Nl — letter numbers */
    if (cp >= 0x16EE && cp <= 0x16F0) return 1; /* Runic numerals */
    if (cp >= 0x2160 && cp <= 0x2188) return 1; /* Roman numerals */
    if (cp >= 0x3007 && cp == 0x3007) return 1; /* Ideographic 0 */
    if (cp >= 0x3021 && cp <= 0x3029) return 1; /* Hangzhou numerals */
    if (cp >= 0x3038 && cp <= 0x303A) return 1;
    /* No — other numbers: vulgar fractions, superscripts, subscripts */
    if (cp >= 0x00B2 && cp <= 0x00B3) return 1;
    if (cp == 0x00B9) return 1;
    if (cp >= 0x00BC && cp <= 0x00BE) return 1;
    if (cp >= 0x0BF0 && cp <= 0x0BF2) return 1;
    if (cp >= 0x1372 && cp <= 0x137C) return 1;
    if (cp >= 0x2070 && cp <= 0x2079) return 1; /* superscripts */
    if (cp >= 0x2080 && cp <= 0x2089) return 1; /* subscripts */
    return 0;
}

/* ── Whitespace ──────────────────────────────────────────────────────────── */

int unicode_is_space(uint32_t cp) {
    /* ASCII whitespace */
    if (cp == 0x09 || cp == 0x0A || cp == 0x0B || cp == 0x0C ||
        cp == 0x0D || cp == 0x20) return 1;
    /* Unicode Zs + other line/paragraph separators */
    if (cp == 0x85) return 1;   /* NEL */
    if (cp == 0xA0) return 1;   /* NBSP */
    if (cp == 0x1680) return 1; /* Ogham space */
    if (cp >= 0x2000 && cp <= 0x200A) return 1; /* various spaces */
    if (cp == 0x2028) return 1; /* line separator */
    if (cp == 0x2029) return 1; /* paragraph separator */
    if (cp == 0x202F) return 1; /* narrow NBSP */
    if (cp == 0x205F) return 1; /* medium mathematical space */
    if (cp == 0x3000) return 1; /* ideographic space */
    if (cp == 0xFEFF) return 1; /* BOM / ZWNBSP (treated as space) */
    return 0;
}

/* ── Punctuation ─────────────────────────────────────────────────────────── */

int unicode_is_punct(uint32_t cp) {
    /* ASCII punctuation */
    if ((cp >= 0x21 && cp <= 0x2F) ||
        (cp >= 0x3A && cp <= 0x40) ||
        (cp >= 0x5B && cp <= 0x60) ||
        (cp >= 0x7B && cp <= 0x7E)) return 1;
    /* Latin-1 punctuation */
    if (cp >= 0x00A1 && cp <= 0x00BF &&
        cp != 0x00A9 && cp != 0x00AE && cp != 0x00B0) return 1;
    /* General punctuation block */
    if (cp >= 0x2010 && cp <= 0x2027) return 1;
    if (cp >= 0x2030 && cp <= 0x205E) return 1;
    /* CJK punctuation */
    if (cp >= 0x3001 && cp <= 0x3003) return 1;
    if (cp >= 0xFE50 && cp <= 0xFE52) return 1;
    if (cp >= 0xFE54 && cp <= 0xFE61) return 1;
    if (cp >= 0xFF01 && cp <= 0xFF0F) return 1;
    if (cp >= 0xFF1A && cp <= 0xFF20) return 1;
    if (cp >= 0xFF3B && cp <= 0xFF3D) return 1;
    if (cp >= 0xFF5B && cp <= 0xFF65) return 1;
    return 0;
}

/* ── Symbol ──────────────────────────────────────────────────────────────── */

int unicode_is_symbol(uint32_t cp) {
    /* Currency symbols */
    if (cp >= 0x0024 && cp == 0x0024) return 1;
    if (cp == 0x00A2 || cp == 0x00A3 || cp == 0x00A4 || cp == 0x00A5) return 1;
    if (cp >= 0x20A0 && cp <= 0x20CF) return 1;
    /* Mathematical operators Sm */
    if (cp >= 0x2200 && cp <= 0x22FF) return 1;
    /* Miscellaneous symbols */
    if (cp >= 0x2300 && cp <= 0x23FF) return 1;
    if (cp >= 0x2400 && cp <= 0x243F) return 1;
    if (cp >= 0x2500 && cp <= 0x257F) return 1; /* Box drawing */
    if (cp >= 0x2580 && cp <= 0x259F) return 1; /* Block elements */
    if (cp >= 0x25A0 && cp <= 0x25FF) return 1; /* Geometric shapes */
    if (cp >= 0x2600 && cp <= 0x26FF) return 1; /* Misc symbols */
    if (cp >= 0x2700 && cp <= 0x27BF) return 1; /* Dingbats */
    if (cp >= 0x1F300 && cp <= 0x1F9FF) return 1; /* Emoji & pictographs */
    return 0;
}

/* ── Combining marks ─────────────────────────────────────────────────────── */

int unicode_is_combining(uint32_t cp) {
    /* Mn — Non-spacing marks */
    if (cp >= 0x0300 && cp <= 0x036F) return 1; /* Combining Diacritical Marks */
    if (cp >= 0x0483 && cp <= 0x0489) return 1;
    if (cp >= 0x0591 && cp <= 0x05BD) return 1;
    if (cp >= 0x05BF && cp == 0x05BF) return 1;
    if (cp >= 0x05C1 && cp <= 0x05C2) return 1;
    if (cp >= 0x05C4 && cp <= 0x05C5) return 1;
    if (cp == 0x05C7) return 1;
    if (cp >= 0x0610 && cp <= 0x061A) return 1;
    if (cp >= 0x064B && cp <= 0x065F) return 1;
    if (cp == 0x0670) return 1;
    if (cp >= 0x06D6 && cp <= 0x06DC) return 1;
    if (cp >= 0x06DF && cp <= 0x06E4) return 1;
    if (cp >= 0x06E7 && cp <= 0x06E8) return 1;
    if (cp >= 0x06EA && cp <= 0x06ED) return 1;
    if (cp >= 0x0711 && cp == 0x0711) return 1;
    if (cp >= 0x0730 && cp <= 0x074A) return 1;
    if (cp >= 0x07A6 && cp <= 0x07B0) return 1;
    if (cp >= 0x07EB && cp <= 0x07F3) return 1;
    if (cp >= 0x0816 && cp <= 0x082D) return 1;
    if (cp >= 0x0900 && cp <= 0x0902) return 1;
    if (cp == 0x093C) return 1;
    if (cp >= 0x0941 && cp <= 0x0948) return 1;
    if (cp == 0x094D) return 1;
    if (cp >= 0x0951 && cp <= 0x0957) return 1;
    if (cp >= 0x0962 && cp <= 0x0963) return 1;
    if (cp == 0x0981) return 1;
    if (cp == 0x09BC) return 1;
    if (cp >= 0x09C1 && cp <= 0x09C4) return 1;
    if (cp == 0x09CD) return 1;
    if (cp >= 0x09E2 && cp <= 0x09E3) return 1;
    if (cp >= 0x1DC0 && cp <= 0x1DFF) return 1; /* Combining Diacritical Supplement */
    if (cp >= 0x20D0 && cp <= 0x20FF) return 1; /* Combining Diacritical for Symbols */
    if (cp >= 0xFE20 && cp <= 0xFE2F) return 1; /* Combining Half Marks */
    return 0;
}

int unicode_is_mark(uint32_t cp) {
    if (unicode_is_combining(cp)) return 1;
    /* Mc — Spacing combining marks */
    if (cp >= 0x0903 && cp <= 0x0903) return 1;
    if (cp >= 0x093E && cp <= 0x0940) return 1;
    if (cp >= 0x0949 && cp <= 0x094C) return 1;
    if (cp >= 0x0982 && cp <= 0x0983) return 1;
    if (cp >= 0x09BE && cp <= 0x09C0) return 1;
    if (cp >= 0x09C7 && cp <= 0x09C8) return 1;
    if (cp >= 0x09CB && cp <= 0x09CC) return 1;
    /* Me — Enclosing marks */
    if (cp >= 0x0488 && cp <= 0x0489) return 1;
    if (cp >= 0x20DD && cp <= 0x20E0) return 1;
    if (cp >= 0x20E2 && cp <= 0x20E4) return 1;
    return 0;
}

/* ── Control ─────────────────────────────────────────────────────────────── */

int unicode_is_control(uint32_t cp) {
    return (cp < 0x20) || (cp >= 0x7F && cp <= 0x9F);
}

/* ── Graphic / print ─────────────────────────────────────────────────────── */

int unicode_is_graphic(uint32_t cp) {
    if (unicode_is_control(cp)) return 0;
    if (cp > UNICODE_MAX_RUNE) return 0;
    if (utf16_is_surrogate(cp)) return 0;
    /* Non-characters */
    if ((cp & 0xFFFF) == 0xFFFE || (cp & 0xFFFF) == 0xFFFF) return 0;
    if (cp >= 0xFDD0 && cp <= 0xFDEF) return 0;
    return 1;
}

int unicode_is_print(uint32_t cp) {
    if (!unicode_is_graphic(cp)) return 0;
    if (unicode_is_space(cp)) return 0;
    return 1;
}

/* ── Identifier properties ───────────────────────────────────────────────── */

int unicode_is_id_start(uint32_t cp) {
    if ((cp >= 'A' && cp <= 'Z') || (cp >= 'a' && cp <= 'z') || cp == '_')
        return 1;
    if (cp < 0x00C0) return 0;
    if (unicode_is_letter(cp)) return 1;
    if (cp >= 0x2070 && cp <= 0x218F) return 1;
    return 0;
}

int unicode_is_id_continue(uint32_t cp) {
    if (unicode_is_id_start(cp)) return 1;
    if (cp >= '0' && cp <= '9') return 1;
    if (unicode_is_digit(cp)) return 1;
    if (unicode_is_combining(cp)) return 1;
    if (cp == 0x00B7) return 1; /* middle dot (used in Catalan) */
    if (cp >= 0x0387 && cp == 0x0387) return 1;
    return 0;
}

int unicode_in_range_latin1(uint32_t cp) { return cp <= UNICODE_MAX_LATIN1; }
int unicode_in_range_bmp   (uint32_t cp) { return cp <= 0xFFFF; }

/* ═══════════════════════════════════════════════════════════════════════════
 *  CASE OPERATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/*
 * Case mapping tables for Latin, Greek, and Cyrillic — the three scripts
 * most common in Xenly identifiers and strings.  Supplementary plane and
 * less-common script case maps fall back to identity (cp unchanged).
 */

uint32_t unicode_to_upper(uint32_t cp) {
    /* ASCII */
    if (cp >= 'a' && cp <= 'z') return cp - 32;
    /* Latin-1 supplement */
    if (cp >= 0x00E0 && cp <= 0x00F6) return cp - 32;
    if (cp >= 0x00F8 && cp <= 0x00FE) return cp - 32;
    if (cp == 0x00FF) return 0x0178; /* ÿ → Ÿ */
    if (cp == 0x00B5) return 0x039C; /* µ → Μ */
    /* Latin Extended-A: alternating lower/upper pairs */
    if (cp >= 0x0101 && cp <= 0x012F && (cp & 1) == 1) return cp - 1;
    if (cp >= 0x0131 && cp <= 0x0137 && (cp & 1) == 1) return cp - 1;
    if (cp >= 0x013A && cp <= 0x0149 && (cp & 1) == 0) return cp - 1;
    if (cp >= 0x014B && cp <= 0x017E && (cp & 1) == 1) return cp - 1;
    if (cp == 0x017F) return 'S';   /* long s */
    /* Greek lowercase → uppercase */
    if (cp >= 0x03B1 && cp <= 0x03C9) return cp - 32;
    if (cp == 0x03C2) return 0x03A3; /* final sigma */
    /* Cyrillic */
    if (cp >= 0x0430 && cp <= 0x044F) return cp - 32;
    if (cp >= 0x0451 && cp <= 0x045F) return cp - 80;
    /* Fullwidth Latin */
    if (cp >= 0xFF41 && cp <= 0xFF5A) return cp - 32;
    /* German ß special mapping (lowercase-only, but handle for completeness) */
    /* Note: ß → SS is multi-char; we return ß unchanged here (single-char map) */
    return cp;
}

uint32_t unicode_to_lower(uint32_t cp) {
    /* ASCII */
    if (cp >= 'A' && cp <= 'Z') return cp + 32;
    /* Latin-1 supplement */
    if (cp >= 0x00C0 && cp <= 0x00D6) return cp + 32;
    if (cp >= 0x00D8 && cp <= 0x00DE) return cp + 32;
    if (cp == 0x0178) return 0x00FF; /* Ÿ → ÿ */
    /* Latin Extended-A */
    if (cp >= 0x0100 && cp <= 0x012E && (cp & 1) == 0) return cp + 1;
    if (cp >= 0x0130 && cp <= 0x0136 && (cp & 1) == 0) return cp + 1;
    if (cp >= 0x0139 && cp <= 0x0148 && (cp & 1) == 1) return cp + 1;
    if (cp >= 0x014A && cp <= 0x017E && (cp & 1) == 0) return cp + 1;
    if (cp == 0x0130) return 0x0069; /* İ → i */
    /* Greek */
    if (cp >= 0x0391 && cp <= 0x03A9) return cp + 32;
    if (cp == 0x03A3) return 0x03C3; /* Σ → σ */
    /* Cyrillic */
    if (cp >= 0x0410 && cp <= 0x042F) return cp + 32;
    if (cp >= 0x0401 && cp <= 0x040F) return cp + 80;
    /* Fullwidth Latin */
    if (cp >= 0xFF21 && cp <= 0xFF3A) return cp + 32;
    return cp;
}

uint32_t unicode_to_title(uint32_t cp) {
    /* For most codepoints titlecase == uppercase */
    /* Titlecase pairs: Dz, Lj, Nj */
    if (cp == 0x01C4 || cp == 0x01C6) return 0x01C5; /* DZ dz → Dz */
    if (cp == 0x01C7 || cp == 0x01C9) return 0x01C8; /* LJ lj → Lj */
    if (cp == 0x01CA || cp == 0x01CC) return 0x01CB; /* NJ nj → Nj */
    if (cp == 0x01F1 || cp == 0x01F3) return 0x01F2; /* DZ dz → Dz (alt) */
    return unicode_to_upper(cp);
}

uint32_t unicode_simple_fold(uint32_t cp) {
    /* Cycle through simple case equivalents, mirrors Go's unicode.SimpleFold */
    if (cp >= 'A' && cp <= 'Z') return cp + 32;
    if (cp >= 'a' && cp <= 'z') return cp - 32;
    if (cp >= 0x00C0 && cp <= 0x00DE && cp != 0x00D7) return cp + 32;
    if (cp >= 0x00E0 && cp <= 0x00FE && cp != 0x00F7) return cp - 32;
    if (unicode_is_upper(cp)) return unicode_to_lower(cp);
    if (unicode_is_lower(cp)) return unicode_to_upper(cp);
    return cp;
}

/* ── String-level case conversion ────────────────────────────────────────── */

size_t utf8_to_upper(const char *src, char *buf, size_t buf_size) {
    if (!src || !buf || buf_size == 0) return 0;
    size_t written = 0, bytes;
    while (*src && written + 4 < buf_size) {
        uint32_t cp = utf8_decode(src, &bytes);
        written += utf8_encode(unicode_to_upper(cp), buf + written);
        src += bytes;
    }
    buf[written] = '\0';
    return written;
}

size_t utf8_to_lower(const char *src, char *buf, size_t buf_size) {
    if (!src || !buf || buf_size == 0) return 0;
    size_t written = 0, bytes;
    while (*src && written + 4 < buf_size) {
        uint32_t cp = utf8_decode(src, &bytes);
        written += utf8_encode(unicode_to_lower(cp), buf + written);
        src += bytes;
    }
    buf[written] = '\0';
    return written;
}

size_t utf8_to_title(const char *src, char *buf, size_t buf_size) {
    if (!src || !buf || buf_size == 0) return 0;
    size_t written = 0, bytes;
    int word_start = 1;
    while (*src && written + 4 < buf_size) {
        uint32_t cp = utf8_decode(src, &bytes);
        uint32_t out;
        if (unicode_is_space(cp) || unicode_is_punct(cp)) {
            word_start = 1;
            out = cp;
        } else if (word_start) {
            out = unicode_to_title(cp);
            word_start = 0;
        } else {
            out = unicode_to_lower(cp);
        }
        written += utf8_encode(out, buf + written);
        src += bytes;
    }
    buf[written] = '\0';
    return written;
}

/* ── Case-insensitive comparison ─────────────────────────────────────────── */

int utf8_equal_fold(const char *a, const char *b) {
    if (!a || !b) return a == b;
    size_t ba, bb;
    while (*a && *b) {
        uint32_t ca = unicode_to_lower(utf8_decode(a, &ba));
        uint32_t cb = unicode_to_lower(utf8_decode(b, &bb));
        if (ca != cb) return 0;
        a += ba; b += bb;
    }
    return *a == '\0' && *b == '\0';
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  GRAPHEME CLUSTERS  (Unicode TR#29 — simplified rules)
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Implemented rules:
 *   GB3  CR × LF
 *   GB4  (Control | CR | LF) ÷
 *   GB5  ÷ (Control | CR | LF)
 *   GB6  L × (L | V | LV | LVT)
 *   GB7  (LV | V) × (V | T)
 *   GB8  (LVT | T) × T
 *   GB9  × (Extend | ZWJ)
 *   GB9b Prepend ×
 *   GB11 ZWJ × Extended_Pictographic
 *   GB999 (Any) ÷ (Any)
 */

/* Grapheme cluster break property (simplified) */
typedef enum {
    GBP_OTHER = 0,
    GBP_CR, GBP_LF, GBP_CONTROL,
    GBP_EXTEND, GBP_ZWJ,
    GBP_PREPEND,
    GBP_SPACINGMARK,
    GBP_L, GBP_V, GBP_T, GBP_LV, GBP_LVT,
    GBP_EXTENDED_PICTOGRAPHIC
} GBProperty;

static GBProperty gbp_of(uint32_t cp) {
    if (cp == 0x000D) return GBP_CR;
    if (cp == 0x000A) return GBP_LF;
    if (unicode_is_control(cp) || cp == 0x200B) return GBP_CONTROL;
    if (cp == 0x200D) return GBP_ZWJ;
    /* Extend: combining marks + Mn category */
    if (unicode_is_combining(cp)) return GBP_EXTEND;
    if (cp >= 0x0903 && cp <= 0x0903) return GBP_SPACINGMARK;
    if (cp >= 0x093E && cp <= 0x0940) return GBP_SPACINGMARK;
    if (cp >= 0x0949 && cp <= 0x094C) return GBP_SPACINGMARK;
    /* Hangul */
    if (cp >= 0x1100 && cp <= 0x115F) return GBP_L;
    if (cp >= 0x1160 && cp <= 0x11A7) return GBP_V;
    if (cp >= 0x11A8 && cp <= 0x11FF) return GBP_T;
    if (cp >= 0xAC00 && cp <= 0xD7A3) {
        uint32_t syl = cp - 0xAC00;
        return (syl % 28 == 0) ? GBP_LV : GBP_LVT;
    }
    /* Extended Pictographic (emoji base characters — broad approximation) */
    if (cp >= 0x1F300 && cp <= 0x1FAFF) return GBP_EXTENDED_PICTOGRAPHIC;
    if (cp >= 0x2600 && cp <= 0x26FF) return GBP_EXTENDED_PICTOGRAPHIC;
    if (cp >= 0x2700 && cp <= 0x27BF) return GBP_EXTENDED_PICTOGRAPHIC;
    return GBP_OTHER;
}

static int gbp_is_break(GBProperty prev, GBProperty cur, int prev_was_zwj) {
    /* GB3 */
    if (prev == GBP_CR && cur == GBP_LF) return 0;
    /* GB4 */
    if (prev == GBP_CR || prev == GBP_LF || prev == GBP_CONTROL) return 1;
    /* GB5 */
    if (cur == GBP_CR || cur == GBP_LF || cur == GBP_CONTROL) return 1;
    /* GB6 */
    if (prev == GBP_L &&
        (cur == GBP_L || cur == GBP_V || cur == GBP_LV || cur == GBP_LVT)) return 0;
    /* GB7 */
    if ((prev == GBP_LV || prev == GBP_V) &&
        (cur == GBP_V || cur == GBP_T)) return 0;
    /* GB8 */
    if ((prev == GBP_LVT || prev == GBP_T) && cur == GBP_T) return 0;
    /* GB9 */
    if (cur == GBP_EXTEND || cur == GBP_ZWJ) return 0;
    /* GB9a */
    if (cur == GBP_SPACINGMARK) return 0;
    /* GB9b */
    if (prev == GBP_PREPEND) return 0;
    /* GB11 */
    if (prev_was_zwj && cur == GBP_EXTENDED_PICTOGRAPHIC) return 0;
    return 1; /* GB999 */
}

size_t grapheme_next_break(const char *str) {
    if (!str || !*str) return 0;
    size_t bytes, total = 0;
    uint32_t cp = utf8_decode(str, &bytes);
    GBProperty prev = gbp_of(cp);
    int prev_zwj = (cp == 0x200D);
    total += bytes;
    str += bytes;

    while (*str) {
        cp = utf8_decode(str, &bytes);
        GBProperty cur = gbp_of(cp);
        if (gbp_is_break(prev, cur, prev_zwj)) break;
        prev_zwj = (cp == 0x200D);
        prev = cur;
        total += bytes;
        str += bytes;
    }
    return total;
}

size_t grapheme_count(const char *str) {
    if (!str) return 0;
    size_t count = 0;
    while (*str) {
        size_t adv = grapheme_next_break(str);
        if (adv == 0) break;
        str += adv;
        count++;
    }
    return count;
}

const char *grapheme_cluster_at(const char *str, size_t index) {
    if (!str) return str;
    for (size_t i = 0; i < index && *str; i++) {
        size_t adv = grapheme_next_break(str);
        if (adv == 0) break;
        str += adv;
    }
    return str;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  DISPLAY WIDTH
 * ═══════════════════════════════════════════════════════════════════════════ */

int unicode_east_asian_width(uint32_t cp) {
    /* Wide (W) and Fullwidth (F) ranges per Unicode EAW data */
    if (cp >= 0x1100 && cp <= 0x115F) return 1;  /* Hangul Jamo */
    if (cp == 0x2329 || cp == 0x232A) return 1;
    if (cp >= 0x2E80 && cp <= 0x303E) return 1;  /* CJK Radicals … CJK symbols */
    if (cp >= 0x3041 && cp <= 0x33BF) return 1;  /* Hiragana … CJK Compatibility */
    if (cp >= 0x33FF && cp <= 0x33FF) return 1;
    if (cp >= 0x3400 && cp <= 0x4DBF) return 1;  /* CJK Ext A */
    if (cp >= 0x4E00 && cp <= 0x9FFF) return 1;  /* CJK Unified Ideographs */
    if (cp >= 0xA000 && cp <= 0xA4CF) return 1;  /* Yi */
    if (cp >= 0xA960 && cp <= 0xA97F) return 1;  /* Hangul Jamo Ext A */
    if (cp >= 0xAC00 && cp <= 0xD7FF) return 1;  /* Hangul Syllables + Jamo Ext B */
    if (cp >= 0xF900 && cp <= 0xFAFF) return 1;  /* CJK Compatibility Ideographs */
    if (cp >= 0xFE10 && cp <= 0xFE19) return 1;
    if (cp >= 0xFE30 && cp <= 0xFE6F) return 1;
    if (cp >= 0xFF00 && cp <= 0xFF60) return 1;  /* Fullwidth Forms */
    if (cp >= 0xFFE0 && cp <= 0xFFE6) return 1;
    if (cp >= 0x1B000 && cp <= 0x1B0FF) return 1;
    if (cp >= 0x1F004 && cp <= 0x1F0CF) return 1;
    if (cp >= 0x1F300 && cp <= 0x1F64F) return 1; /* Emoji */
    if (cp >= 0x20000 && cp <= 0x2FFFD) return 1; /* CJK Ext B-F */
    if (cp >= 0x30000 && cp <= 0x3FFFD) return 1; /* CJK Ext G+ */
    return 0;
}

size_t utf8_display_width(const char *str) {
    if (!str) return 0;
    size_t width = 0, bytes;
    while (*str) {
        uint32_t cp = utf8_decode(str, &bytes);
        str += bytes;
        if (unicode_is_combining(cp) || unicode_is_control(cp)) continue;
        width += unicode_east_asian_width(cp) ? 2 : 1;
    }
    return width;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  NORMALIZATION HELPERS
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Full NFC/NFD requires a complete Unicode decomposition database.
 * We implement:
 *  — Canonical combining class (CCC) for common combining marks
 *  — A selected set of canonical compositions for the most common
 *    pre-composed characters (Latin diacritics, covering 90%+ of usage)
 *  — Canonical reordering of combining sequences
 *  — Starter-compose pass for NFC
 */

/* CCC table: only non-zero classes are listed */
typedef struct { uint32_t lo, hi; uint8_t ccc; } CCCRange;

static const CCCRange ccc_ranges[] = {
    { 0x0300, 0x0314, 230 }, { 0x0315, 0x0315, 232 },
    { 0x0316, 0x0319, 220 }, { 0x031A, 0x031A, 232 },
    { 0x031B, 0x031B, 216 }, { 0x031C, 0x0320, 220 },
    { 0x0321, 0x0322, 202 }, { 0x0323, 0x0326, 220 },
    { 0x0327, 0x0328, 202 }, { 0x0329, 0x0333, 220 },
    { 0x0334, 0x0338, 1   }, { 0x0339, 0x033C, 220 },
    { 0x033D, 0x0344, 230 }, { 0x0345, 0x0345, 240 },
    { 0x0346, 0x034E, 230 }, { 0x034F, 0x034F, 0   },
    { 0x0350, 0x0352, 230 }, { 0x0353, 0x0356, 220 },
    { 0x0357, 0x0357, 230 }, { 0x0358, 0x035C, 232 },
    { 0x035D, 0x035E, 234 }, { 0x035F, 0x035F, 233 },
    { 0x0360, 0x0361, 234 }, { 0x0362, 0x0362, 233 },
    { 0x0363, 0x036F, 230 },
    /* Combining marks in other scripts — approximate */
    { 0x0483, 0x0487, 230 },
    { 0x0591, 0x05BD, 220 }, { 0x05BF, 0x05BF, 23 },
    { 0x0610, 0x061A, 230 }, { 0x064B, 0x0652, 27 },
    { 0x0670, 0x0670, 35 },
    { 0x0900, 0x0902, 0 }, /* Devanagari viramas etc — CCC 0 for starters */
    { 0x093C, 0x093C, 7 }, { 0x0941, 0x0948, 0 }, { 0x094D, 0x094D, 9 },
    { 0x1DC0, 0x1DFF, 230 },
    { 0x20D0, 0x20DC, 230 }, { 0x20E1, 0x20E1, 230 },
    { 0xFE20, 0xFE2F, 230 },
    { 0, 0, 0 }
};

uint8_t unicode_canonical_class(uint32_t cp) {
    for (int i = 0; ccc_ranges[i].lo != 0; i++) {
        if (cp >= ccc_ranges[i].lo && cp <= ccc_ranges[i].hi)
            return ccc_ranges[i].ccc;
    }
    return 0;
}

/* Canonical composition pairs — most common Latin pre-composed characters */
typedef struct { uint32_t starter, combining, composed; } CompPair;

static const CompPair comp_pairs[] = {
    /* Grave: à á â ã ä å æ ç */
    { 0x0041, 0x0300, 0x00C0 }, { 0x0041, 0x0301, 0x00C1 },
    { 0x0041, 0x0302, 0x00C2 }, { 0x0041, 0x0303, 0x00C3 },
    { 0x0041, 0x0308, 0x00C4 }, { 0x0041, 0x030A, 0x00C5 },
    { 0x0043, 0x0327, 0x00C7 },
    { 0x0045, 0x0300, 0x00C8 }, { 0x0045, 0x0301, 0x00C9 },
    { 0x0045, 0x0302, 0x00CA }, { 0x0045, 0x0308, 0x00CB },
    { 0x0049, 0x0300, 0x00CC }, { 0x0049, 0x0301, 0x00CD },
    { 0x0049, 0x0302, 0x00CE }, { 0x0049, 0x0308, 0x00CF },
    { 0x004E, 0x0303, 0x00D1 },
    { 0x004F, 0x0300, 0x00D2 }, { 0x004F, 0x0301, 0x00D3 },
    { 0x004F, 0x0302, 0x00D4 }, { 0x004F, 0x0303, 0x00D5 },
    { 0x004F, 0x0308, 0x00D6 },
    { 0x0055, 0x0300, 0x00D9 }, { 0x0055, 0x0301, 0x00DA },
    { 0x0055, 0x0302, 0x00DB }, { 0x0055, 0x0308, 0x00DC },
    { 0x0059, 0x0301, 0x00DD },
    /* lowercase */
    { 0x0061, 0x0300, 0x00E0 }, { 0x0061, 0x0301, 0x00E1 },
    { 0x0061, 0x0302, 0x00E2 }, { 0x0061, 0x0303, 0x00E3 },
    { 0x0061, 0x0308, 0x00E4 }, { 0x0061, 0x030A, 0x00E5 },
    { 0x0063, 0x0327, 0x00E7 },
    { 0x0065, 0x0300, 0x00E8 }, { 0x0065, 0x0301, 0x00E9 },
    { 0x0065, 0x0302, 0x00EA }, { 0x0065, 0x0308, 0x00EB },
    { 0x0069, 0x0300, 0x00EC }, { 0x0069, 0x0301, 0x00ED },
    { 0x0069, 0x0302, 0x00EE }, { 0x0069, 0x0308, 0x00EF },
    { 0x006E, 0x0303, 0x00F1 },
    { 0x006F, 0x0300, 0x00F2 }, { 0x006F, 0x0301, 0x00F3 },
    { 0x006F, 0x0302, 0x00F4 }, { 0x006F, 0x0303, 0x00F5 },
    { 0x006F, 0x0308, 0x00F6 },
    { 0x0075, 0x0300, 0x00F9 }, { 0x0075, 0x0301, 0x00FA },
    { 0x0075, 0x0302, 0x00FB }, { 0x0075, 0x0308, 0x00FC },
    { 0x0079, 0x0301, 0x00FD }, { 0x0079, 0x0308, 0x00FF },
    /* Latin Extended-A most-used */
    { 0x0041, 0x0304, 0x0100 }, { 0x0061, 0x0304, 0x0101 },
    { 0x0041, 0x0306, 0x0102 }, { 0x0061, 0x0306, 0x0103 },
    { 0x0041, 0x0328, 0x0104 }, { 0x0061, 0x0328, 0x0105 },
    { 0x0043, 0x0301, 0x0106 }, { 0x0063, 0x0301, 0x0107 },
    { 0x0043, 0x0302, 0x0108 }, { 0x0063, 0x0302, 0x0109 },
    { 0x0043, 0x0307, 0x010A }, { 0x0063, 0x0307, 0x010B },
    { 0x0043, 0x030C, 0x010C }, { 0x0063, 0x030C, 0x010D },
    { 0x0044, 0x030C, 0x010E }, { 0x0064, 0x030C, 0x010F },
    { 0x0045, 0x0304, 0x0112 }, { 0x0065, 0x0304, 0x0113 },
    { 0x0045, 0x0306, 0x0114 }, { 0x0065, 0x0306, 0x0115 },
    { 0x0045, 0x0307, 0x0116 }, { 0x0065, 0x0307, 0x0117 },
    { 0x0045, 0x0328, 0x0118 }, { 0x0065, 0x0328, 0x0119 },
    { 0x0045, 0x030C, 0x011A }, { 0x0065, 0x030C, 0x011B },
    { 0, 0, 0 }
};

uint32_t unicode_compose_pair(uint32_t starter, uint32_t combining) {
    for (int i = 0; comp_pairs[i].starter != 0; i++) {
        if (comp_pairs[i].starter  == starter &&
            comp_pairs[i].combining == combining)
            return comp_pairs[i].composed;
    }
    return 0;
}

/* NFD: decompose pre-composed characters to base + combining */
static size_t nfd_decompose(uint32_t cp, uint32_t *out) {
    /* Check composition table in reverse */
    for (int i = 0; comp_pairs[i].starter != 0; i++) {
        if (comp_pairs[i].composed == cp) {
            out[0] = comp_pairs[i].starter;
            out[1] = comp_pairs[i].combining;
            return 2;
        }
    }
    out[0] = cp;
    return 1;
}

size_t utf8_nfd(const char *src, char *buf, size_t buf_size) {
    if (!src || !buf || buf_size == 0) return 0;
    /* Build rune buffer, decompose, sort by CCC */
    size_t rune_cap = 256;
    uint32_t *runes = (uint32_t *)malloc(rune_cap * sizeof(uint32_t));
    if (!runes) { buf[0] = '\0'; return 0; }
    size_t rune_len = 0, bytes;
    while (*src) {
        uint32_t cp = utf8_decode(src, &bytes);
        src += bytes;
        uint32_t decomp[2];
        size_t n = nfd_decompose(cp, decomp);
        if (rune_len + n > rune_cap) {
            rune_cap = rune_cap * 2 + n;
            runes = (uint32_t *)realloc(runes, rune_cap * sizeof(uint32_t));
            if (!runes) { buf[0] = '\0'; return 0; }
        }
        for (size_t i = 0; i < n; i++)
            runes[rune_len++] = decomp[i];
    }
    /* Canonical reordering: bubble sort by CCC (stable, O(n²) acceptable for text) */
    for (size_t i = 1; i < rune_len; i++) {
        uint8_t ccc_i = unicode_canonical_class(runes[i]);
        if (ccc_i == 0) continue;
        for (size_t j = i; j > 0; j--) {
            uint8_t ccc_j = unicode_canonical_class(runes[j-1]);
            if (ccc_j == 0 || ccc_j <= ccc_i) break;
            uint32_t tmp = runes[j]; runes[j] = runes[j-1]; runes[j-1] = tmp;
            ccc_i = ccc_j;
        }
    }
    /* Encode back to UTF-8 */
    size_t written = 0;
    for (size_t i = 0; i < rune_len && written + 4 < buf_size; i++)
        written += utf8_encode(runes[i], buf + written);
    buf[written] = '\0';
    free(runes);
    return written;
}

size_t utf8_nfc(const char *src, char *buf, size_t buf_size) {
    if (!src || !buf || buf_size == 0) return 0;
    /* First decompose to NFD */
    size_t nfd_cap = strlen(src) * 4 + 1;
    char *nfd_buf = (char *)malloc(nfd_cap);
    if (!nfd_buf) { buf[0] = '\0'; return 0; }
    utf8_nfd(src, nfd_buf, nfd_cap);
    /* Canonical composition pass */
    size_t rune_cap = 256;
    uint32_t *runes = (uint32_t *)malloc(rune_cap * sizeof(uint32_t));
    if (!runes) { free(nfd_buf); buf[0] = '\0'; return 0; }
    size_t rune_len = 0, bytes;
    const char *p = nfd_buf;
    while (*p) {
        uint32_t cp = utf8_decode(p, &bytes); p += bytes;
        if (rune_len >= rune_cap) {
            rune_cap *= 2;
            runes = (uint32_t *)realloc(runes, rune_cap * sizeof(uint32_t));
            if (!runes) { free(nfd_buf); buf[0] = '\0'; return 0; }
        }
        runes[rune_len++] = cp;
    }
    free(nfd_buf);
    /* Compose: for each starter, attempt to absorb following combiners */
    size_t out_len = 0;
    for (size_t i = 0; i < rune_len; ) {
        if (unicode_canonical_class(runes[i]) != 0 || i == rune_len - 1) {
            if (out_len >= rune_cap) { rune_cap *= 2; runes = (uint32_t *)realloc(runes, rune_cap * sizeof(uint32_t)); }
            if (out_len < rune_cap) runes[out_len++] = runes[i++];
            continue;
        }
        uint32_t starter = runes[i];
        size_t j = i + 1;
        while (j < rune_len) {
            uint8_t ccc = unicode_canonical_class(runes[j]);
            /* Check for blocking */
            int blocked = 0;
            for (size_t k = i + 1; k < j; k++) {
                uint8_t ccc_k = unicode_canonical_class(runes[k]);
                if (ccc_k >= ccc || ccc_k == 0) { blocked = 1; break; }
            }
            if (!blocked) {
                uint32_t comp = unicode_compose_pair(starter, runes[j]);
                if (comp != 0) {
                    starter = comp;
                    /* Remove runes[j] by shifting */
                    for (size_t k = j; k < rune_len - 1; k++) runes[k] = runes[k+1];
                    rune_len--;
                    continue;
                }
            }
            j++;
        }
        if (out_len >= rune_cap) { rune_cap *= 2; runes = (uint32_t *)realloc(runes, rune_cap * sizeof(uint32_t)); }
        if (out_len < rune_cap) runes[out_len++] = starter;
        i++;
    }
    /* Encode */
    size_t written = 0;
    for (size_t i = 0; i < out_len && written + 4 < buf_size; i++)
        written += utf8_encode(runes[i], buf + written);
    buf[written] = '\0';
    free(runes);
    return written;
}
