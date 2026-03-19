#ifndef RAYLIB_PAINT_H
#define RAYLIB_PAINT_H

#include <raylib.h>
#include "linked_list.h"
#include "globals.h"


// Utility: get line index in buffer
int get_line_index(DoublyLinkedList *buffer, Line *line);
// Measure width of first 'len' characters using proper font metrics
// 
int get_total_content_height(DoublyLinkedList *buffer, int line_height);

float MeasureTextLen(const char *text, size_t len, Font font, float fontSize);
// Draw line numbers gutter
int draw_line_numbers(DoublyLinkedList *buffer, Font font, float fontSize, int line_height, int current_line_idx, int win_h);
// Draw linked-list buffer with cursor and scrolling
void draw_buffer(DoublyLinkedList *buffer, Font font, float fontSize, int line_height, int gutter_width, int win_w, int win_h);
// Update scroll so current line is visible with padding
void update_scroll(DoublyLinkedList *buffer, int line_height, int win_h);

void handle_mouse_click(DoublyLinkedList *buffer, Font font, float fontSize, int line_height, int gutter_width, int win_h);



#endif // RAYLIB_PAINT_H