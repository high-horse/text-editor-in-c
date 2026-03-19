#ifndef UTF8_H
#define UTF8_H

#include <stddef.h>
#include <stdint.h>


bool is_valid_utf8(const char *str);

// Get byte length of UTF-8 character starting at s[0] (1-4, or 0 if invalid)
static inline int utf8_char_len(const char *s) {
    unsigned char c = (unsigned char) s[0];
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1; // Invalid, treat as single byte
}

// Count UTF-8 characters in string (not bytes)
size_t utf8_strlen(const char *s);

// Convert character index to byte offset
size_t utf8_char_to_byte(const char *s, size_t char_idx);

// Convert byte offset to character index  
size_t utf8_byte_to_char(const char *s, size_t byte_idx);

// Get UTF-32 codepoint at character index (for display/measurement)
uint32_t utf8_char_at(const char *s, size_t char_idx);

// Insert UTF-32 codepoint at character position, returns new byte length
size_t utf8_insert_char(char *dest, size_t dest_cap, const char *src, size_t src_len, 
                        size_t char_pos, uint32_t codepoint);

// Delete n characters at char position, returns new byte length
size_t utf8_delete_chars(char *dest, const char *src, size_t src_len, size_t char_pos, size_t n_chars);


#endif