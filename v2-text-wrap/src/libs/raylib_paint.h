#ifndef RAYLIB_PAINT_H
#define RAYLIB_PAINT_H

#include <raylib.h>
#include "linked_list.h"
#include "globals.h"

// Utility: get line index in buffer
int get_line_index(DoublyLinkedList *buffer, Line *line);

// Measure width of first 'len' characters using proper font metrics
float MeasureTextLen(const char *text, size_t len, Font font, float fontSize);

// Measure substring width
float MeasureTextSub(const char *text, int start, int len, Font font, float fontSize);

// Calculate visual lines needed for text wrapping
int calculate_visual_lines(const char *text, Font font, float fontSize, float max_width);

// Get visual line and column for cursor position
void get_visual_position(Line *line, int cursor_col, Font font, float fontSize, 
                         float max_width, int *out_visual_line, int *out_visual_col);

// Get total content height accounting for wrapped lines
int get_total_content_height(DoublyLinkedList *buffer, int line_height);

// Draw line numbers gutter (updated signature)
int draw_line_numbers(DoublyLinkedList *buffer, Font font, float fontSize, int line_height, 
                       int current_line_idx, int win_h, float text_area_width);

// Draw wrapped text with continuation indicators
void draw_wrapped_text(const char *text, int start_x, int start_y, Font font, 
                       float fontSize, int line_height, float max_width,
                       int scroll_offset, int win_h, Color text_color);

// Draw linked-list buffer with text wrapping
void draw_buffer(DoublyLinkedList *buffer, Font font, float fontSize, int line_height, 
                 int gutter_width, int win_w, int win_h);

// Update scroll with wrapping support (updated signature)
void update_scroll(DoublyLinkedList *buffer, int line_height, int win_h, Font font, 
                   float fontSize, int gutter_width, int win_w);

// Handle mouse click with wrapping support (updated signature)
void handle_mouse_click(DoublyLinkedList *buffer, Font font, float fontSize,
                        int line_height, int gutter_width, int win_h, int win_w);

#endif // RAYLIB_PAINT_H