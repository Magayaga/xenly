#include "unicode.h"
#include <string.h>

// Decode a single UTF-8 character
uint32_t utf8_decode(const char *str, size_t *bytes) {
    const unsigned char *s = (const unsigned char *)str;
    uint32_t cp;
    
    if (s[0] < 0x80) {
        // 1-byte (ASCII)
        *bytes = 1;
        return s[0];
    } else if ((s[0] & 0xE0) == 0xC0) {
        // 2-byte
        *bytes = 2;
        cp = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
        return cp;
    } else if ((s[0] & 0xF0) == 0xE0) {
        // 3-byte
        *bytes = 3;
        cp = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        return cp;
    } else if ((s[0] & 0xF8) == 0xF0) {
        // 4-byte
        *bytes = 4;
        cp = ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12) | 
             ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        return cp;
    }
    
    // Invalid UTF-8
    *bytes = 1;
    return 0xFFFD; // Replacement character
}

// Encode a Unicode codepoint to UTF-8
size_t utf8_encode(uint32_t cp, char *out) {
    if (cp < 0x80) {
        // 1-byte
        out[0] = (char)cp;
        return 1;
    } else if (cp < 0x800) {
        // 2-byte
        out[0] = (char)(0xC0 | (cp >> 6));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    } else if (cp < 0x10000) {
        // 3-byte
        out[0] = (char)(0xE0 | (cp >> 12));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    } else if (cp < 0x110000) {
        // 4-byte
        out[0] = (char)(0xF0 | (cp >> 18));
        out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[3] = (char)(0x80 | (cp & 0x3F));
        return 4;
    }
    
    // Invalid codepoint
    return 0;
}

// Get length of UTF-8 string in characters
size_t utf8_strlen(const char *str) {
    size_t len = 0;
    size_t bytes;
    
    while (*str) {
        utf8_decode(str, &bytes);
        str += bytes;
        len++;
    }
    
    return len;
}

// Check if byte is continuation byte
int utf8_is_continuation(unsigned char byte) {
    return (byte & 0xC0) == 0x80;
}

// Check if codepoint is valid identifier start
// Follows Unicode ID_Start property (simplified)
int unicode_is_id_start(uint32_t cp) {
    // ASCII letters
    if ((cp >= 'A' && cp <= 'Z') || (cp >= 'a' && cp <= 'z') || cp == '_')
        return 1;
    
    // Common Unicode letter ranges
    if (cp >= 0x00C0 && cp <= 0x00D6) return 1; // Latin extended
    if (cp >= 0x00D8 && cp <= 0x00F6) return 1;
    if (cp >= 0x00F8 && cp <= 0x02FF) return 1;
    if (cp >= 0x0370 && cp <= 0x037D) return 1; // Greek
    if (cp >= 0x037F && cp <= 0x1FFF) return 1;
    if (cp >= 0x200C && cp <= 0x200D) return 1; // Zero-width joiners
    if (cp >= 0x2070 && cp <= 0x218F) return 1;
    if (cp >= 0x2C00 && cp <= 0x2FEF) return 1;
    if (cp >= 0x3001 && cp <= 0xD7FF) return 1; // CJK
    if (cp >= 0xF900 && cp <= 0xFDCF) return 1;
    if (cp >= 0xFDF0 && cp <= 0xFFFD) return 1;
    if (cp >= 0x10000 && cp <= 0xEFFFF) return 1;
    
    return 0;
}

// Check if codepoint is valid identifier continuation
int unicode_is_id_continue(uint32_t cp) {
    // ID_Start characters
    if (unicode_is_id_start(cp))
        return 1;
    
    // Digits
    if (cp >= '0' && cp <= '9')
        return 1;
    
    // Common combining marks and modifiers
    if (cp >= 0x0300 && cp <= 0x036F) return 1; // Combining marks
    if (cp >= 0x0483 && cp <= 0x0487) return 1;
    if (cp >= 0x0591 && cp <= 0x05BD) return 1;
    if (cp >= 0x0600 && cp <= 0x0603) return 1;
    if (cp >= 0x0610 && cp <= 0x061A) return 1;
    if (cp >= 0x064B && cp <= 0x0669) return 1;
    if (cp >= 0x0670 && cp <= 0x06D3) return 1;
    
    return 0;
}

// Advance pointer by one UTF-8 character
const char *utf8_next(const char *str) {
    size_t bytes;
    utf8_decode(str, &bytes);
    return str + bytes;
}

// Get the nth UTF-8 character
const char *utf8_char_at(const char *str, size_t index) {
    size_t bytes;
    
    for (size_t i = 0; i < index && *str; i++) {
        utf8_decode(str, &bytes);
        str += bytes;
    }
    
    return str;
}
