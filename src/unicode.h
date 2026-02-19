#ifndef UNICODE_H
#define UNICODE_H

#include <stdint.h>
#include <stddef.h>

// UTF-8 encoding/decoding utilities

// Decode a single UTF-8 character from string
// Returns the Unicode codepoint and updates *bytes with number of bytes consumed
uint32_t utf8_decode(const char *str, size_t *bytes);

// Encode a Unicode codepoint to UTF-8
// Returns number of bytes written (1-4), or 0 on error
size_t utf8_encode(uint32_t codepoint, char *out);

// Get length of UTF-8 string in characters (not bytes)
size_t utf8_strlen(const char *str);

// Check if a byte is a UTF-8 continuation byte (10xxxxxx)
int utf8_is_continuation(unsigned char byte);

// Check if a Unicode codepoint is a valid identifier start character
int unicode_is_id_start(uint32_t cp);

// Check if a Unicode codepoint is a valid identifier continuation character
int unicode_is_id_continue(uint32_t cp);

// Advance pointer by one UTF-8 character
const char *utf8_next(const char *str);

// Get the nth UTF-8 character from a string
const char *utf8_char_at(const char *str, size_t index);

#endif // UNICODE_H
