/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 * 
 * It is available for the Linux and macOS operating systems.
 *
 */
#ifndef XLY_RT_H
#define XLY_RT_H

/*
 * xly_rt  —  Xenly compiled-binary runtime  (v0.1.0)
 *
 * Every Xenly value at runtime is an opaque pointer to an XlyVal on the heap.
 * The runtime owns all allocation.  Compiled code never touches struct
 * internals directly; it only calls the functions declared here.
 *
 * Calling convention: System V AMD64 / AAPCS64.
 *
 * New in v0.1.0 (systems programming tier):
 *   xly_ptr_read / xly_ptr_write  — raw memory access for mmap regions
 *   xly_syscall                   — raw Linux syscall wrapper
 *   xly_str_len                   — O(1) string length as XlyVal*
 *   xly_int                       — truncate number to integer XlyVal*
 *   xly_str_slice                 — substring [start,end)
 *   xly_str_byte                  — get byte value of char at index
 *   xly_num_bits                  — reinterpret double bits as uint64
 *   xly_bits_num                  — reinterpret uint64 bits as double
 *   xly_write_stderr              — write to fd 2 (error logging)
 *   xly_perror                    — print C strerror to stderr
 */

#include <stddef.h>   /* size_t   */
#include <stdint.h>   /* int64_t  */

/* ── opaque value pointer ──────────────────────────────────────────────────── */
typedef struct XlyVal XlyVal;

/* ── constructors ──────────────────────────────────────────────────────────── */
XlyVal *xly_num(double n);
XlyVal *xly_str(const char *s);     /* copies s            */
XlyVal *xly_bool(int b);            /* 0 or 1              */
XlyVal *xly_null(void);
XlyVal *xly_make_variant(XlyVal *tag, XlyVal **fields, int nfields);

/* ── arithmetic / comparison  (both args must be valid XlyVal*) ────────────── */
XlyVal *xly_add(XlyVal *a, XlyVal *b);   /* num+num  or  str concat */
XlyVal *xly_sub(XlyVal *a, XlyVal *b);
XlyVal *xly_mul(XlyVal *a, XlyVal *b);
XlyVal *xly_div(XlyVal *a, XlyVal *b);
XlyVal *xly_mod(XlyVal *a, XlyVal *b);
XlyVal *xly_neg(XlyVal *a);              /* unary minus */
XlyVal *xly_not(XlyVal *a);             /* logical not */

/* comparison → xly_bool */
XlyVal *xly_eq(XlyVal *a, XlyVal *b);
XlyVal *xly_neq(XlyVal *a, XlyVal *b);
XlyVal *xly_lt(XlyVal *a, XlyVal *b);
XlyVal *xly_gt(XlyVal *a, XlyVal *b);
XlyVal *xly_lte(XlyVal *a, XlyVal *b);
XlyVal *xly_gte(XlyVal *a, XlyVal *b);

/* ── truthiness  (C int, not XlyVal*) ──────────────────────────────────────── */
int     xly_truthy(XlyVal *v);

/* ── I/O ────────────────────────────────────────────────────────────────────── */
void    xly_print(XlyVal **vals, size_t n);
XlyVal *xly_input(XlyVal *prompt);

/* ── string operations ──────────────────────────────────────────────────────── */
char   *xly_to_cstr(XlyVal *v);          /* caller must free()          */
XlyVal *xly_typeof(XlyVal *v);
char   *value_to_string(XlyVal *v);      /* caller must free()          */

/* NEW: systems-level string helpers */
XlyVal *xly_str_len(XlyVal *s);          /* length of string as number  */
XlyVal *xly_str_slice(XlyVal *s, XlyVal *start, XlyVal *end); /* [start,end) */
XlyVal *xly_str_byte(XlyVal *s, XlyVal *idx);  /* byte value (0-255) at idx */
XlyVal *xly_str_concat(XlyVal *a, XlyVal *b);  /* always concatenate         */
XlyVal *xly_str_find(XlyVal *haystack, XlyVal *needle); /* first index or -1 */
XlyVal *xly_str_repeat(XlyVal *s, XlyVal *n);  /* repeat string n times      */

/* ── numeric type operations ────────────────────────────────────────────────── */
XlyVal *xly_int(XlyVal *v);              /* truncate double → integer   */
XlyVal *xly_abs(XlyVal *v);             /* absolute value              */
XlyVal *xly_floor(XlyVal *v);
XlyVal *xly_ceil(XlyVal *v);
XlyVal *xly_round(XlyVal *v);
XlyVal *xly_sqrt(XlyVal *v);
XlyVal *xly_pow(XlyVal *base, XlyVal *exp);
XlyVal *xly_min(XlyVal *a, XlyVal *b);
XlyVal *xly_max(XlyVal *a, XlyVal *b);
XlyVal *xly_clamp(XlyVal *v, XlyVal *lo, XlyVal *hi);

/* ── raw memory / systems programming ──────────────────────────────────────── */
/*
 * xly_ptr_read(ptr_num, offset, size)
 *   Read `size` bytes from (ptr + offset) → XlyVal* string.
 *   ptr_num is a number holding a uintptr_t (from sys.mmap or sys.mmap_read).
 */
XlyVal *xly_ptr_read(XlyVal *ptr_num, XlyVal *offset, XlyVal *size);

/*
 * xly_ptr_write(ptr_num, offset, data_str)
 *   Write string bytes to (ptr + offset).  Returns bytes written as number.
 */
XlyVal *xly_ptr_write(XlyVal *ptr_num, XlyVal *offset, XlyVal *data);

/*
 * xly_ptr_read_u8/u16/u32/u64(ptr_num, offset)
 *   Read a fixed-width integer from raw memory → number.
 *   These are zero-overhead for tight systems loops (drivers, parsers).
 */
XlyVal *xly_ptr_read_u8 (XlyVal *ptr, XlyVal *off);
XlyVal *xly_ptr_read_u16(XlyVal *ptr, XlyVal *off);
XlyVal *xly_ptr_read_u32(XlyVal *ptr, XlyVal *off);
XlyVal *xly_ptr_read_u64(XlyVal *ptr, XlyVal *off);

/*
 * xly_ptr_write_u8/u16/u32/u64(ptr_num, offset, value)
 *   Write a fixed-width integer to raw memory.
 */
XlyVal *xly_ptr_write_u8 (XlyVal *ptr, XlyVal *off, XlyVal *val);
XlyVal *xly_ptr_write_u16(XlyVal *ptr, XlyVal *off, XlyVal *val);
XlyVal *xly_ptr_write_u32(XlyVal *ptr, XlyVal *off, XlyVal *val);
XlyVal *xly_ptr_write_u64(XlyVal *ptr, XlyVal *off, XlyVal *val);

/*
 * xly_syscall(nr, a1, a2, a3, a4, a5, a6)
 *   Direct Linux syscall.  All args are numbers (cast to long).
 *   Returns syscall result as number.  Negative → errno.
 *   This is for advanced users who know the Linux ABI.
 */
XlyVal *xly_syscall(XlyVal *nr, XlyVal *a1, XlyVal *a2,
                    XlyVal *a3, XlyVal *a4, XlyVal *a5, XlyVal *a6);

/*
 * Double ↔ uint64 bit reinterpretation.
 * Required for bit manipulation of floating-point values (IEEE 754 tricks,
 * NaN boxing, fast inverse sqrt, etc.).
 */
XlyVal *xly_num_bits(XlyVal *v);   /* double → uint64 bit pattern as number */
XlyVal *xly_bits_num(XlyVal *v);   /* uint64 → double reinterpret            */

/* ── error I/O ──────────────────────────────────────────────────────────────── */
void    xly_write_stderr(const char *msg);      /* write C string to fd 2   */
XlyVal *xly_eprint(XlyVal *v);                  /* print value to stderr    */
XlyVal *xly_perror(XlyVal *msg);                /* perror-style output      */

/* ── value utilities ─────────────────────────────────────────────────────────── */
XlyVal *value_clone(XlyVal *v);
void    value_destroy(XlyVal *v);

/* ── array operations ───────────────────────────────────────────────────────── */
XlyVal *xly_array_create(XlyVal **elems, size_t n);
size_t  xly_array_len(XlyVal *arr);
XlyVal *xly_array_get(XlyVal *arr, size_t idx);
void    xly_array_set(XlyVal *arr, size_t idx, XlyVal *val);
XlyVal *xly_array_push(XlyVal *arr, XlyVal *val);
XlyVal *xly_array_pop(XlyVal *arr);
XlyVal *xly_array_slice(XlyVal *arr, XlyVal *start, XlyVal *end);
XlyVal *xly_array_concat(XlyVal *a, XlyVal *b);
XlyVal *xly_array_reverse(XlyVal *arr);
XlyVal *xly_array_contains(XlyVal *arr, XlyVal *val);

/* ── module dispatch ─────────────────────────────────────────────────────────── */
XlyVal *xly_call_module(const char *mod, const char *fn,
                         XlyVal **args, size_t argc);

/* ── indexing ────────────────────────────────────────────────────────────────── */
XlyVal *xly_index(XlyVal *collection, XlyVal *index_val);

/* ── process exit ────────────────────────────────────────────────────────────── */
void    xly_exit(int code);   /* calls exit() */

/* ── first-class function values ─────────────────────────────────────────────── */
/* Wrap a raw C function pointer as a VAL_FUNCTION XlyVal* */
XlyVal *xly_make_fn(void *fp);
/* Create a closure: fn ptr + captured-variable environment array */
XlyVal *xly_make_closure(void *fp, XlyVal **env, int env_size);
/* Get the environment pointer from a closure XlyVal* */
XlyVal **xly_closure_env(XlyVal *closure);
/* Call a VAL_FUNCTION XlyVal* with given args */
XlyVal *xly_call_fnval(XlyVal *fn_val, XlyVal **args, int argc);

/* ── object / instance operations ───────────────────────────────────────────── */
XlyVal  *xly_obj_new(void);                                    /* create empty object     */
void     xly_obj_set(XlyVal *obj, const char *key, XlyVal *val); /* set field              */
XlyVal  *xly_obj_get(XlyVal *obj, const char *key);            /* get field (null if miss) */
XlyVal  *xly_obj_call(XlyVal *obj, const char *method, XlyVal **args, int argc);
XlyVal  *xly_this(void);                               /* current method receiver */

/* ── mutable closure capture cells ──────────────────────────────────────────── */
XlyVal **xly_make_cell(XlyVal *initial);   /* allocate heap cell for mutable capture */
XlyVal  *xly_cell_get(XlyVal **cell);      /* dereference cell                       */
void     xly_cell_set(XlyVal **cell, XlyVal *val); /* write through cell              */

/* ══════════════════════════════════════════════════════════════════════════════
 * UNICODE / UTF-8 RUNTIME API
 *
 * Every Xenly string is a NUL-terminated UTF-8 byte sequence.  The functions
 * below expose the full unicode.h surface through the XlyVal* ABI so that
 * compiled Xenly programs can call them without any C interop boilerplate.
 *
 * Naming convention:
 *   xly_uni_*   — Unicode character / codepoint operations (XlyVal* in/out)
 *   xly_utf8_*  — UTF-8 string operations
 *   xly_str_*   — already-existing byte-level string ops (kept for compat)
 *
 * All functions follow the same error contract as the rest of xly_rt:
 *   — Invalid type arguments return xly_null() or xly_num(0) / xly_str("").
 *   — Out-of-bounds indices return xly_null().
 *   — Strings are always valid UTF-8 on output (sanitized).
 * ══════════════════════════════════════════════════════════════════════════════ */

/* ── UTF-8 string length / indexing ─────────────────────────────────────────── */

/* xly_utf8_len(s) → number: codepoint count (not byte count) */
XlyVal *xly_utf8_len(XlyVal *s);

/* xly_utf8_char_at(s, index) → string: one UTF-8 character at codepoint index */
XlyVal *xly_utf8_char_at(XlyVal *s, XlyVal *index);

/* xly_utf8_slice(s, start, end) → string: codepoints [start,end) */
XlyVal *xly_utf8_slice(XlyVal *s, XlyVal *start, XlyVal *end);

/* xly_utf8_byte_offset(s, index) → number: byte offset of Nth codepoint */
XlyVal *xly_utf8_byte_offset(XlyVal *s, XlyVal *index);

/* xly_utf8_is_valid(s) → bool: 1 if s is valid UTF-8 */
XlyVal *xly_utf8_is_valid(XlyVal *s);

/* xly_utf8_sanitize(s) → string: replace invalid sequences with U+FFFD */
XlyVal *xly_utf8_sanitize(XlyVal *s);

/* xly_utf8_reverse(s) → string: reverse by codepoints */
XlyVal *xly_utf8_reverse(XlyVal *s);

/* xly_utf8_display_width(s) → number: terminal column width */
XlyVal *xly_utf8_display_width(XlyVal *s);

/* xly_utf8_contains_rune(s, cp) → bool: true if codepoint cp appears in s */
XlyVal *xly_utf8_contains_rune(XlyVal *s, XlyVal *cp);

/* xly_utf8_count_rune(s, cp) → number: occurrences of codepoint in s */
XlyVal *xly_utf8_count_rune(XlyVal *s, XlyVal *cp);

/* xly_utf8_index_rune(s, cp) → number: byte offset of first cp, or -1 */
XlyVal *xly_utf8_index_rune(XlyVal *s, XlyVal *cp);

/* xly_utf8_equal_fold(a, b) → bool: case-insensitive Unicode comparison */
XlyVal *xly_utf8_equal_fold(XlyVal *a, XlyVal *b);

/* ── Unicode codepoint operations ────────────────────────────────────────────── */

/* xly_uni_codepoint_at(s, index) → number: codepoint value at index */
XlyVal *xly_uni_codepoint_at(XlyVal *s, XlyVal *index);

/* xly_uni_from_codepoint(cp) → string: UTF-8 string from codepoint number */
XlyVal *xly_uni_from_codepoint(XlyVal *cp);

/* ── Unicode character classification ────────────────────────────────────────── */

/* All accept a codepoint number or a 1-character string */
XlyVal *xly_uni_is_letter (XlyVal *cp);  /* Lu Ll Lt Lm Lo */
XlyVal *xly_uni_is_upper  (XlyVal *cp);  /* Lu */
XlyVal *xly_uni_is_lower  (XlyVal *cp);  /* Ll */
XlyVal *xly_uni_is_title  (XlyVal *cp);  /* Lt */
XlyVal *xly_uni_is_digit  (XlyVal *cp);  /* Nd — decimal digit */
XlyVal *xly_uni_is_number (XlyVal *cp);  /* Nd Nl No */
XlyVal *xly_uni_is_space  (XlyVal *cp);  /* Unicode whitespace */
XlyVal *xly_uni_is_punct  (XlyVal *cp);  /* Punctuation categories */
XlyVal *xly_uni_is_symbol (XlyVal *cp);  /* Symbol categories */
XlyVal *xly_uni_is_mark   (XlyVal *cp);  /* Combining marks */
XlyVal *xly_uni_is_control(XlyVal *cp);  /* Control characters */
XlyVal *xly_uni_is_graphic(XlyVal *cp);  /* Printable incl. space */
XlyVal *xly_uni_is_print  (XlyVal *cp);  /* Printable excl. space */
XlyVal *xly_uni_digit_value(XlyVal *cp); /* number: 0-9, or -1 */

/* ── Case conversion ─────────────────────────────────────────────────────────── */

/* xly_uni_to_upper/lower/title(cp) → number: case-mapped codepoint */
XlyVal *xly_uni_to_upper(XlyVal *cp);
XlyVal *xly_uni_to_lower(XlyVal *cp);
XlyVal *xly_uni_to_title(XlyVal *cp);

/* xly_uni_simple_fold(cp) → number: next simple case-fold equivalent */
XlyVal *xly_uni_simple_fold(XlyVal *cp);

/* String-level case (operate on whole UTF-8 strings) */
XlyVal *xly_utf8_to_upper(XlyVal *s);
XlyVal *xly_utf8_to_lower(XlyVal *s);
XlyVal *xly_utf8_to_title(XlyVal *s);

/* ── Grapheme clusters (Unicode TR#29) ───────────────────────────────────────── */

/* xly_grapheme_count(s) → number: number of user-perceived characters */
XlyVal *xly_grapheme_count(XlyVal *s);

/* xly_grapheme_at(s, index) → string: Nth grapheme cluster */
XlyVal *xly_grapheme_at(XlyVal *s, XlyVal *index);

/* xly_grapheme_next_break(s) → number: bytes until next cluster boundary */
XlyVal *xly_grapheme_next_break(XlyVal *s);

/* ── Normalization ────────────────────────────────────────────────────────────── */

/* xly_utf8_nfc(s) → string: NFC-normalized UTF-8 */
XlyVal *xly_utf8_nfc(XlyVal *s);

/* xly_utf8_nfd(s) → string: NFD-normalized UTF-8 */
XlyVal *xly_utf8_nfd(XlyVal *s);

/* ── UTF-16 interop ───────────────────────────────────────────────────────────── */

/* xly_utf16_encode(cp) → array: [high, low] surrogate pair, or [cp] if BMP */
XlyVal *xly_utf16_encode(XlyVal *cp);

/* xly_utf16_decode(high, low) → number: codepoint from surrogate pair */
XlyVal *xly_utf16_decode(XlyVal *high, XlyVal *low);

/* xly_uni_is_surrogate(cp) → bool */
XlyVal *xly_uni_is_surrogate(XlyVal *cp);

#endif /* XLY_RT_H */
