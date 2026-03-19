#include "raylib_paint.h"
#include "globals.h"
#include <raylib.h>
#include <stdio.h>
#include <string.h>

// Utility: get line index in buffer
int get_line_index(DoublyLinkedList *buffer, Line *line) {
    int idx = 0;
    Line *cur = buffer->head;
    while (cur && cur != line) {
        cur = cur->next;
        idx++;
    }
    return idx;
}

// Measure width of first 'len' characters using proper font metrics
float MeasureTextLen(const char *text, size_t len, Font font, float fontSize) {
    if (!text || len == 0) return 0;
    char tmp[len + 1];
    strncpy(tmp, text, len);
    tmp[len] = '\0';
    Vector2 size = MeasureTextEx(font, tmp, fontSize, 1.0f);
    return size.x;
}

// Draw line numbers gutter
int draw_line_numbers(DoublyLinkedList *buffer, Font font, float fontSize, int line_height, int current_line_idx, int win_h) {
    int line_count = 0;
    Line *cur = buffer->head;
    while (cur) {
        line_count++;
        cur = cur->next;
    }
    
    // Calculate gutter width based on max line number digits
    char num_str[16];
    snprintf(num_str, sizeof(num_str), "%d", line_count);
    int gutter_width = MeasureTextEx(font, num_str, fontSize, 1.0f).x + 20;
    
    // Draw gutter background
    DrawRectangle(0, 0, gutter_width, win_h, (Color){ 40, 40, 40, 255 });
    DrawLine(gutter_width, 0, gutter_width, win_h, (Color){ 60, 60, 60, 255 });
    
    // Draw line numbers
    int y = PADDING_Y - scroll_y;
    int idx = 0;
    cur = buffer->head;
    
    while (cur && y < win_h) {
        if (y + line_height > 0) {  // Only draw if visible
            snprintf(num_str, sizeof(num_str), "%d", idx + 1);
            
            // Highlight current line number
            Color num_color = (idx == current_line_idx) ? CURSOR_COLOR : LINE_NUM_COLOR;
            float num_width = MeasureTextEx(font, num_str, fontSize, 1.0f).x;
            
            DrawTextEx(font, num_str, 
                      (Vector2){ gutter_width - num_width - 8, (float)y }, 
                      fontSize, 1.0f, num_color);
        }
        y += line_height;
        idx++;
        cur = cur->next;
    }
    
    // Return gutter width for main drawing
    return gutter_width + 10;
}

// Draw linked-list buffer with cursor and scrolling
void draw_buffer(DoublyLinkedList *buffer, Font font, float fontSize, int line_height, int gutter_width, int win_w, int win_h) {
    if (!buffer) return;

    int y = PADDING_Y - scroll_y;
    int idx = 0;
    Line *cur = buffer->head;
    int current_line_idx = get_line_index(buffer, buffer->current_line);

    // Draw text content
    while (cur && y < win_h) {
        if (y + line_height > 0) {  // Only draw visible lines
            // Optional: subtle highlight for current line background
            if (idx == current_line_idx) {
                DrawRectangle(gutter_width, y - 2, win_w - gutter_width - 10, line_height, (Color){ 50, 50, 50, 100 });
            }
            
            DrawTextEx(font, cur->text, 
                      (Vector2){ (float)gutter_width, (float)y }, 
                      fontSize, 1.0f, TEXT_COLOR);
        }
        y += line_height;
        idx++;
        cur = cur->next;
    }

    // Draw blinking cursor
    if (buffer->current_line && cursor_visible) {
        int line_idx = get_line_index(buffer, buffer->current_line);
        float cursor_x = gutter_width + MeasureTextLen(buffer->current_line->text, buffer->cursor_col, font, fontSize);
        int cursor_y = line_idx * line_height - scroll_y + PADDING_Y;
        
        // Draw cursor as vertical bar
        DrawRectangle((int)cursor_x, cursor_y, 2, line_height - 4, CURSOR_COLOR);
    }
}

// Update scroll so current line is visible with padding
void update_scroll(DoublyLinkedList *buffer, int line_height, int win_h) {
    if (!buffer->current_line) return;
    int line_y = get_line_index(buffer, buffer->current_line) * line_height;
    int visible_height = win_h - PADDING_Y * 2;
    
    if (line_y - scroll_y < 0) {
        scroll_y = line_y - PADDING_Y;
    } else if (line_y - scroll_y > visible_height - line_height) {
        scroll_y = line_y - visible_height + line_height + PADDING_Y;
    }
    
    // Clamp scroll
    if (scroll_y < 0) scroll_y = 0;
}

void handle_mouse_click(DoublyLinkedList *buffer, Font font, float fontSize,
                        int line_height, int gutter_width, int win_h) {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    
    Vector2 mouse = GetMousePosition();
    int status_height = 25;
    
    // Ignore clicks on gutter or status bar
    if (mouse.x < gutter_width || mouse.y > win_h - status_height) return;
    
    // Calculate target line
    int clicked_line = (mouse.y + scroll_y - PADDING_Y) / line_height;
    
    // Clamp to valid line range
    int line_count = 0;
    Line *tmp = buffer->head;
    while (tmp) { line_count++; tmp = tmp->next; }
    
    if (clicked_line < 0) clicked_line = 0;
    if (clicked_line >= line_count) clicked_line = line_count - 1;
    
    // Find target line node
    Line *target = buffer->head;
    for (int i = 0; i < clicked_line && target; i++) {
        target = target->next;
    }
    
    if (!target) return;
    
    // Calculate target column by measuring text widths
    float rel_x = mouse.x - gutter_width;
    size_t len = strlen(target->text);
    size_t col = 0;
    
    // Binary search for closest character position
    size_t low = 0, high = len;
    while (low < high) {
        size_t mid = (low + high + 1) / 2;
        float width = MeasureTextLen(target->text, mid, font, fontSize);
        
        if (width < rel_x) {
            low = mid;
        } else {
            high = mid - 1;
        }
    }
    col = low;
    
    // Check if closer to next character
    if (col < len) {
        float w1 = MeasureTextLen(target->text, col, font, fontSize);
        float w2 = MeasureTextLen(target->text, col + 1, font, fontSize);
        if (rel_x - w1 > w2 - rel_x) {
            col++;
        }
    }
    
    // Update cursor
    buffer->current_line = target;
    buffer->cursor_col = col;
    
    // Reset blink
    cursor_visible = true;
    cursor_blink_timer = 0.0f;
}