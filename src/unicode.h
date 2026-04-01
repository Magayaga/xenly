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
 * XENLY Unicode Support — unicode.h
 *
 * Comprehensive UTF-8 / UTF-16 codec, Unicode character classification,
 * case folding, grapheme-cluster segmentation, display-width, and
 * normalization utilities.
 *
 * Inspired by Go's unicode, unicode/utf8, and unicode/utf16 packages.
 * Xenly source files are UTF-8 encoded by default.  Individual characters
 * are represented as 32-bit Unicode codepoints (analogous to Go runes /
 * int32).  Source code, strings, and identifiers all support the full
 * Unicode range U+0000 – U+10FFFF.
 */
#ifndef UNICODE_H
#define UNICODE_H

#include <stdint.h>
#include <stddef.h>

/* ── Constants ───────────────────────────────────────────────────────────── */
#define UNICODE_MAX_RUNE     UINT32_C(0x10FFFF)
#define UNICODE_REPLACEMENT  UINT32_C(0xFFFD)
#define UNICODE_RUNE_ERROR   UINT32_C(0xFFFD)
#define UNICODE_BOM          UINT32_C(0xFEFF)
#define UNICODE_MAX_ASCII    UINT32_C(0x007F)
#define UNICODE_MAX_LATIN1   UINT32_C(0x00FF)
#define UTF8_MAX_BYTES       4

/* ── UTF-8 codec ─────────────────────────────────────────────────────────── */
uint32_t    utf8_decode        (const char *str, size_t *bytes);
uint32_t    utf8_decode_rune   (const char *str, size_t *bytes, int *valid);
size_t      utf8_encode        (uint32_t cp, char *out);
size_t      utf8_strlen        (const char *str);
size_t      utf8_strlen_n      (const char *str, size_t n);
size_t      utf8_byte_len      (const char *str, size_t n);
int         utf8_is_valid      (const char *str);
int         utf8_is_valid_n    (const char *str, size_t n);
int         utf8_is_continuation(unsigned char byte);
int         utf8_full_rune     (const char *str, size_t n);
const char *utf8_next          (const char *str);
const char *utf8_prev          (const char *str, const char *start);
const char *utf8_char_at       (const char *str, size_t index);
size_t      utf8_index_byte    (const char *str, size_t index);
size_t      utf8_slice         (const char *src, size_t start, size_t end,
                                char *buf, size_t buf_size);
int         utf8_contains_rune (const char *str, uint32_t cp);
size_t      utf8_index_rune    (const char *str, uint32_t cp);
size_t      utf8_rune_count_n  (const char *str, size_t n);
size_t      utf8_count_rune    (const char *str, uint32_t cp);
size_t      utf8_reverse       (const char *src, char *buf, size_t buf_size);
size_t      utf8_sanitize      (const char *src, char *buf, size_t buf_size);
int         utf8_equal_fold    (const char *a, const char *b);

/* ── UTF-16 codec ────────────────────────────────────────────────────────── */
int         utf16_is_surrogate      (uint32_t cp);
int         utf16_is_high_surrogate (uint32_t cp);
int         utf16_is_low_surrogate  (uint32_t cp);
int         utf16_encode_pair       (uint32_t cp, uint16_t *high, uint16_t *low);
uint32_t    utf16_decode_pair       (uint16_t high, uint16_t low);

/* ── Unicode character properties ────────────────────────────────────────── */
int unicode_is_letter    (uint32_t cp);
int unicode_is_upper     (uint32_t cp);
int unicode_is_lower     (uint32_t cp);
int unicode_is_title     (uint32_t cp);
int unicode_is_digit     (uint32_t cp);
int unicode_is_number    (uint32_t cp);
int unicode_is_space     (uint32_t cp);
int unicode_is_punct     (uint32_t cp);
int unicode_is_symbol    (uint32_t cp);
int unicode_is_mark      (uint32_t cp);
int unicode_is_control   (uint32_t cp);
int unicode_is_graphic   (uint32_t cp);
int unicode_is_print     (uint32_t cp);
int unicode_is_combining (uint32_t cp);
int unicode_is_id_start  (uint32_t cp);
int unicode_is_id_continue(uint32_t cp);
int unicode_in_range_latin1(uint32_t cp);
int unicode_in_range_bmp (uint32_t cp);
int unicode_digit_value  (uint32_t cp);

/* ── Case operations ─────────────────────────────────────────────────────── */
uint32_t unicode_to_upper   (uint32_t cp);
uint32_t unicode_to_lower   (uint32_t cp);
uint32_t unicode_to_title   (uint32_t cp);
uint32_t unicode_simple_fold(uint32_t cp);
size_t   utf8_to_upper      (const char *src, char *buf, size_t buf_size);
size_t   utf8_to_lower      (const char *src, char *buf, size_t buf_size);
size_t   utf8_to_title      (const char *src, char *buf, size_t buf_size);

/* ── Grapheme clusters (Unicode TR#29) ───────────────────────────────────── */
size_t      grapheme_next_break  (const char *str);
size_t      grapheme_count       (const char *str);
const char *grapheme_cluster_at  (const char *str, size_t index);

/* ── Display width ───────────────────────────────────────────────────────── */
int    unicode_east_asian_width(uint32_t cp);
size_t utf8_display_width      (const char *str);

/* ── Normalization helpers ───────────────────────────────────────────────── */
uint8_t  unicode_canonical_class(uint32_t cp);
uint32_t unicode_compose_pair   (uint32_t starter, uint32_t combining);
size_t   utf8_nfc               (const char *src, char *buf, size_t buf_size);
size_t   utf8_nfd               (const char *src, char *buf, size_t buf_size);

#endif /* UNICODE_H */
