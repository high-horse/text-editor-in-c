#include "raylib_paint.h"
#include "globals.h"
#include <raylib.h>
#include <stdio.h>
#include <string.h>

// --- Visual Wrap Configuration ---
#define WRAP_INDENT 20          // Indentation for wrapped continuation lines
#define WRAP_INDICATOR "↳ "     // Visual indicator for wrapped lines (or use "│ " or "↪ ")
#define WRAP_INDICATOR_COLOR (Color){100, 100, 100, 255}

// Structure to track visual line info for wrapped lines
typedef struct {
    Line *original_line;        // Pointer to original line in buffer
    int start_col;              // Starting column in original line
    int length;                 // Length of this visual segment
    bool is_continuation;       // True if this is a wrapped continuation
} VisualLine;

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

// Measure width of substring from start with given length
float MeasureTextSub(const char *text, int start, int len, Font font, float fontSize) {
    if (!text || len <= 0) return 0;
    int text_len = strlen(text);
    if (start >= text_len) return 0;
    
    int actual_len = (start + len > text_len) ? (text_len - start) : len;
    char tmp[actual_len + 1];
    strncpy(tmp, text + start, actual_len);
    tmp[actual_len] = '\0';
    
    Vector2 size = MeasureTextEx(font, tmp, fontSize, 1.0f);
    return size.x;
}

int get_total_content_height(DoublyLinkedList *buffer, int line_height) {
    // For wrapped text, we need to calculate based on visual lines
    // This is a simplified version - actual implementation needs font info
    int lines = 0;
    Line *cur = buffer->head;
    while (cur) {
        lines++;
        cur = cur->next;
    }
    return lines * line_height;
}


// Calculate how many visual lines a logical line needs with word wrapping
int calculate_visual_lines(const char *text, Font font, float fontSize, float max_width) {
    if (!text || strlen(text) == 0) return 1;
    
    float text_width = MeasureTextEx(font, text, fontSize, 1.0f).x;
    if (text_width <= max_width) return 1;
    
    // More accurate calculation: binary search for break points
    int visual_lines = 0;
    int pos = 0;
    int len = strlen(text);
    
    while (pos < len) {
        visual_lines++;
        
        // Find how many characters fit in this segment
        int low = 0, high = len - pos;
        int fit_count = 0;
        
        while (low <= high) {
            int mid = (low + high) / 2;
            float width = MeasureTextLen(text + pos, mid, font, fontSize);
            
            if (width <= max_width) {
                fit_count = mid;
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        
        if (fit_count == 0) fit_count = 1; // Force progress
        
        // Try to break at word boundary
        if (fit_count < len - pos) {
            // Look for space to break
            int break_pos = fit_count;
            for (int i = fit_count; i > 0; i--) {
                if (text[pos + i] == ' ' || text[pos + i] == '\t') {
                    break_pos = i;
                    break;
                }
            }
            pos += break_pos;
            // Skip trailing space
            while (pos < len && (text[pos] == ' ' || text[pos] == '\t')) pos++;
        } else {
            pos += fit_count;
        }
    }
    
    return visual_lines > 0 ? visual_lines : 1;
}


// Find the visual line and column for a given cursor position
void get_visual_position(Line *line, int cursor_col, Font font, float fontSize, 
                         float max_width, int *out_visual_line, int *out_visual_col) {
    if (!line || !line->text) {
        *out_visual_line = 0;
        *out_visual_col = 0;
        return;
    }
    
    const char *text = line->text;
    int len = strlen(text);
    if (cursor_col > len) cursor_col = len;
    
    int visual_line = 0;
    int pos = 0;
    int visual_col = cursor_col;
    
    while (pos < cursor_col) {
        // Find how many characters fit in current segment
        int low = 0, high = cursor_col - pos;
        int fit_count = 0;
        
        while (low <= high) {
            int mid = (low + high) / 2;
            float width = MeasureTextLen(text + pos, mid, font, fontSize);
            if (width <= max_width) {
                fit_count = mid;
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        
        if (fit_count == 0) fit_count = 1;
        
        // Try word boundary
        if (pos + fit_count < len && pos + fit_count < cursor_col) {
            for (int i = fit_count; i > 0; i--) {
                if (text[pos + i] == ' ' || text[pos + i] == '\t') {
                    fit_count = i;
                    break;
                }
            }
        }
        
        if (pos + fit_count >= cursor_col) {
            visual_col = cursor_col - pos;
            break;
        }
        
        pos += fit_count;
        while (pos < len && (text[pos] == ' ' || text[pos] == '\t')) pos++;
        visual_line++;
        visual_col = cursor_col - pos;
    }
    
    *out_visual_line = visual_line;
    *out_visual_col = visual_col;
}

// Draw line numbers gutter with support for wrapped lines
int draw_line_numbers(DoublyLinkedList *buffer, Font font, float fontSize, int line_height, 
                       int current_line_idx, int win_h, float text_area_width) {
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
    
    // Draw line numbers accounting for wrapped lines
    int y = PADDING_Y - scroll_y;
    int idx = 0;
    cur = buffer->head;
    
    while (cur && y < win_h) {
        int visual_lines = calculate_visual_lines(cur->text, font, fontSize, text_area_width);
        
        // Only draw line number for first visual line of each logical line
        if (y + line_height > 0) {
            snprintf(num_str, sizeof(num_str), "%d", idx + 1);
            
            // Highlight current line number
            Color num_color = (idx == current_line_idx) ? CURSOR_COLOR : LINE_NUM_COLOR;
            float num_width = MeasureTextEx(font, num_str, fontSize, 1.0f).x;
            
            // Center the number vertically if line wraps
            int num_y = y;
            if (visual_lines > 1) {
                num_y = y + (visual_lines * line_height - line_height) / 2;
            }
            
            DrawTextEx(font, num_str, 
                      (Vector2){ gutter_width - num_width - 8, (float)num_y }, 
                      fontSize, 1.0f, num_color);
        }
        
        y += visual_lines * line_height;
        idx++;
        cur = cur->next;
    }
    
    return gutter_width + 10;
}

// Draw wrapped text with continuation indicators
void draw_wrapped_text(const char *text, int start_x, int start_y, Font font, 
                       float fontSize, int line_height, float max_width,
                       int scroll_offset __attribute__((unused)), int win_h, Color text_color) {
    if (!text || strlen(text) == 0) return;
    
    int len = strlen(text);
    int pos = 0;
    int visual_line = 0;
    float y = start_y;
    int fit_count = 0;  // Declare here to be accessible throughout the function
    
    while (pos < len && y < win_h + line_height) {
        // Calculate indent and indicator for this line
        int indent = (visual_line > 0) ? WRAP_INDENT : 0;
        bool is_continuation = (visual_line > 0);
        
        // Find how many characters fit in available width
        float indicator_width = is_continuation ? MeasureTextEx(font, WRAP_INDICATOR, fontSize, 1.0f).x : 0;
        float available_width = max_width - indent - indicator_width;
        
        // Binary search for fit count
        int low = 0, high = len - pos;
        fit_count = 0;
        
        while (low <= high) {
            int mid = (low + high) / 2;
            float width = MeasureTextLen(text + pos, mid, font, fontSize);
            if (width <= available_width) {
                fit_count = mid;
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        
        if (fit_count == 0) fit_count = 1;
        
        // Word boundary check (only if not at end)
        if (pos + fit_count < len) {
            // Look for last space within fit range
            int break_at = fit_count;
            for (int i = fit_count; i > 0; i--) {
                if (text[pos + i] == ' ' || text[pos + i] == '\t') {
                    break_at = i;
                    break;
                }
            }
            
            // Only use word break if we found one and it's not the first char
            if (break_at > 0 && break_at < fit_count) {
                fit_count = break_at;
            }
        }
        
        // Only draw if visible
        if (y + line_height > 0) {
            // Draw continuation indicator
            if (is_continuation) {
                DrawTextEx(font, WRAP_INDICATOR, 
                          (Vector2){ (float)start_x, (float)y }, 
                          fontSize, 1.0f, WRAP_INDICATOR_COLOR);
            }
            
            // Draw text segment
            char segment[fit_count + 1];
            strncpy(segment, text + pos, fit_count);
            segment[fit_count] = '\0';
            
            int draw_x = start_x + indent + (is_continuation ? indicator_width : 0);
            DrawTextEx(font, segment, 
                      (Vector2){ (float)draw_x, (float)y }, 
                      fontSize, 1.0f, text_color);
        }
        
        // Move to next segment
        pos += fit_count;
        // Skip trailing spaces for next line
        while (pos < len && (text[pos] == ' ' || text[pos] == '\t')) pos++;
        
        visual_line++;
        y += line_height;
    }
}

// Draw linked-list buffer with text wrapping and visual indicators
void draw_buffer(DoublyLinkedList *buffer, Font font, float fontSize, int line_height, 
                 int gutter_width, int win_w, int win_h) {
    if (!buffer) return;

    float text_area_width = win_w - gutter_width - PADDING_X - 10;
    int y = PADDING_Y - scroll_y;
    int idx = 0;
    Line *cur = buffer->head;
    int current_line_idx = get_line_index(buffer, buffer->current_line);

    // Draw text content with wrapping
    while (cur && y < win_h) {
        int visual_lines = calculate_visual_lines(cur->text, font, fontSize, text_area_width);
        
        if (y + (visual_lines * line_height) > 0) {  // Only draw if any part visible
            // Highlight current line background (span all visual lines)
            if (idx == current_line_idx) {
                DrawRectangle(gutter_width, y - 2, win_w - gutter_width - 10, 
                           visual_lines * line_height, (Color){ 50, 50, 50, 100 });
            }
            
            // Draw wrapped text
            draw_wrapped_text(cur->text, gutter_width, y, font, fontSize, 
                            line_height, text_area_width, scroll_y, win_h, TEXT_COLOR);
        }
        
        y += visual_lines * line_height;
        idx++;
        cur = cur->next;
    }

    // Draw blinking cursor (positioned correctly for wrapped text)
    if (buffer->current_line && cursor_visible) {
        int line_idx = get_line_index(buffer, buffer->current_line);
        
        // Calculate Y position accounting for wrapped lines before cursor
        int y_pos = PADDING_Y - scroll_y;
        Line *tmp = buffer->head;
        for (int i = 0; i < line_idx && tmp; i++) {
            y_pos += calculate_visual_lines(tmp->text, font, fontSize, text_area_width) * line_height;
            tmp = tmp->next;
        }
        
        // Get visual line and column within wrapped line
        int visual_line, visual_col;
        get_visual_position(buffer->current_line, buffer->cursor_col, font, fontSize, 
                           text_area_width, &visual_line, &visual_col);
        
        // Adjust Y for visual line within wrapped line
        y_pos += visual_line * line_height;
        
        // Calculate X position by finding start of current visual line
        const char *text = buffer->current_line->text;
        int len = strlen(text);
        int pos = 0;
        int current_visual = 0;
        int visual_start_col = 0;
        
        // Find the starting column of current visual line
        while (pos < len && current_visual < visual_line) {
            float available_width = text_area_width - (current_visual > 0 ? WRAP_INDENT + MeasureTextEx(font, WRAP_INDICATOR, fontSize, 1.0f).x : 0);
            
            int fit_count = 0;
            int low = 0, high = len - pos;
            while (low <= high) {
                int mid = (low + high) / 2;
                float width = MeasureTextLen(text + pos, mid, font, fontSize);
                if (width <= available_width) {
                    fit_count = mid;
                    low = mid + 1;
                } else {
                    high = mid - 1;
                }
            }
            if (fit_count == 0) fit_count = 1;
            
            // Word boundary
            if (pos + fit_count < len) {
                for (int i = fit_count; i > 0; i--) {
                    if (text[pos + i] == ' ' || text[pos + i] == '\t') {
                        fit_count = i;
                        break;
                    }
                }
            }
            
            pos += fit_count;
            while (pos < len && (text[pos] == ' ' || text[pos] == '\t')) pos++;
            current_visual++;
        }
        visual_start_col = pos;
        
        // Calculate X position
        int indent = (visual_line > 0) ? WRAP_INDENT + MeasureTextEx(font, WRAP_INDICATOR, fontSize, 1.0f).x : 0;
        float cursor_x = gutter_width + indent + 
                        MeasureTextLen(buffer->current_line->text + visual_start_col, 
                                      visual_col, font, fontSize);
        
        // Draw cursor as vertical bar
        DrawRectangle((int)cursor_x, y_pos, 2, line_height - 4, CURSOR_COLOR);
    }
}

// Update scroll so current line is visible with padding (accounts for wrapped lines)
void update_scroll(DoublyLinkedList *buffer, int line_height, int win_h, Font font, 
                   float fontSize, int gutter_width, int win_w) {
    if (!buffer->current_line) return;
    
    float text_area_width = win_w - gutter_width - PADDING_X - 10;
    int line_idx = get_line_index(buffer, buffer->current_line);
    
    // Calculate total height up to current line
    int total_y = 0;
    Line *cur = buffer->head;
    for (int i = 0; i < line_idx && cur; i++) {
        total_y += calculate_visual_lines(cur->text, font, fontSize, text_area_width) * line_height;
        cur = cur->next;
    }
    
    // Add offset for visual line within current wrapped line
    int visual_line, visual_col;
    get_visual_position(buffer->current_line, buffer->cursor_col, font, fontSize, 
                       text_area_width, &visual_line, &visual_col);
    total_y += visual_line * line_height;
    
    int visible_height = win_h - PADDING_Y * 2;
    
    if (total_y - scroll_y < 0) {
        scroll_y = total_y - PADDING_Y;
    } else if (total_y - scroll_y > visible_height - line_height) {
        scroll_y = total_y - visible_height + line_height + PADDING_Y;
    }
    
    // Clamp scroll
    if (scroll_y < 0) scroll_y = 0;
    
    // Calculate max scroll based on total visual height
    int total_content_height = 0;
    cur = buffer->head;
    while (cur) {
        total_content_height += calculate_visual_lines(cur->text, font, fontSize, text_area_width) * line_height;
        cur = cur->next;
    }
    
    int max_scroll = total_content_height - visible_height;
    if (max_scroll < 0) max_scroll = 0;
    if (scroll_y > max_scroll) scroll_y = max_scroll;
}

void handle_mouse_click(DoublyLinkedList *buffer, Font font, float fontSize,
                        int line_height, int gutter_width, int win_h, int win_w) {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    
    Vector2 mouse = GetMousePosition();
    int status_height = 25;
    float text_area_width = win_w - gutter_width - PADDING_X - 10;
    
    // Ignore clicks on gutter or status bar
    if (mouse.x < gutter_width || mouse.y > win_h - status_height) return;
    
    // Calculate which visual line was clicked
    int clicked_y = (int)(mouse.y + scroll_y - PADDING_Y);
    int current_visual_y = 0;
    int clicked_line = -1;
    int visual_line_in_logical = 0;
    
    Line *tmp = buffer->head;
    int line_idx = 0;
    
    while (tmp) {
        int visual_lines = calculate_visual_lines(tmp->text, font, fontSize, text_area_width);
        int line_height_total = visual_lines * line_height;
        
        if (clicked_y >= current_visual_y && clicked_y < current_visual_y + line_height_total) {
            clicked_line = line_idx;
            visual_line_in_logical = (clicked_y - current_visual_y) / line_height;
            break;
        }
        
        current_visual_y += line_height_total;
        line_idx++;
        tmp = tmp->next;
    }
    
    // Clamp to valid line range
    int line_count = 0;
    tmp = buffer->head;
    while (tmp) { line_count++; tmp = tmp->next; }
    
    if (clicked_line < 0) clicked_line = line_count - 1;
    if (clicked_line >= line_count) clicked_line = line_count - 1;
    
    // Find target line node
    Line *target = buffer->head;
    for (int i = 0; i < clicked_line && target; i++) {
        target = target->next;
    }
    
    if (!target) return;
    
    // Calculate column within wrapped line
    float rel_x = mouse.x - gutter_width;
    int indent = (visual_line_in_logical > 0) ? WRAP_INDENT + MeasureTextEx(font, WRAP_INDICATOR, fontSize, 1.0f).x : 0;
    rel_x -= indent;
    
    // Find which segment of the wrapped line we're in
    const char *text = target->text;
    int len = strlen(text);
    int pos = 0;
    int current_visual = 0;
    size_t col = 0;
    
    while (pos < len && current_visual < visual_line_in_logical) {
        // Skip to next visual line
        float available_width = text_area_width - (current_visual > 0 ? WRAP_INDENT + MeasureTextEx(font, WRAP_INDICATOR, fontSize, 1.0f).x : 0);
        
        int fit_count = 0;
        int low = 0, high = len - pos;
        while (low <= high) {
            int mid = (low + high) / 2;
            float width = MeasureTextLen(text + pos, mid, font, fontSize);
            if (width <= available_width) {
                fit_count = mid;
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        if (fit_count == 0) fit_count = 1;
        
        // Word boundary
        if (pos + fit_count < len) {
            for (int i = fit_count; i > 0; i--) {
                if (text[pos + i] == ' ' || text[pos + i] == '\t') {
                    fit_count = i;
                    break;
                }
            }
        }
        
        pos += fit_count;
        while (pos < len && (text[pos] == ' ' || text[pos] == '\t')) pos++;
        current_visual++;
    }
    
    // Now find column within this segment
    int segment_start = pos;
    int remaining = len - pos;
    int segment_len = 0;
    
    // Calculate segment length
    float available_width = text_area_width - (visual_line_in_logical > 0 ? WRAP_INDENT + MeasureTextEx(font, WRAP_INDICATOR, fontSize, 1.0f).x : 0);
    int low = 0, high = remaining;
    while (low <= high) {
        int mid = (low + high) / 2;
        float width = MeasureTextLen(text + pos, mid, font, fontSize);
        if (width <= available_width) {
            segment_len = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    if (segment_len == 0) segment_len = remaining;
    
    // Binary search for closest character position within segment
    low = 0; 
    high = segment_len;
    while (low < high) {
        size_t mid = (low + high + 1) / 2;
        float width = MeasureTextLen(text + segment_start, mid, font, fontSize);
        
        if (width < rel_x) {
            low = mid;
        } else {
            high = mid - 1;
        }
    }
    col = segment_start + low;
    
    // Check if closer to next character
    if (col < (size_t)len) {
        float w1 = MeasureTextLen(text, col - segment_start, font, fontSize);
        float w2 = MeasureTextLen(text, col - segment_start + 1, font, fontSize);
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