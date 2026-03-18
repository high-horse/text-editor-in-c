#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <raylib.h>
#include <time.h>
#include "libs/linked_list.h"
#include "libs/font_data.h"

#define WIN_W 800
#define WIN_H 600
#define FPS 60
#define LINE_SPACING 4
#define FONT_SIZE 18
#define PADDING_X 20
#define PADDING_Y 20
#define CURSOR_BLINK_RATE 0.53f  // Cursor blink interval in seconds

// Global scroll offset
int scroll_y = 0;

// Cursor blink state
float cursor_blink_timer = 0.0f;
bool cursor_visible = true;

// Color scheme - modern dark theme
#define BG_COLOR        (Color){ 30, 30, 30, 255 }      // Dark gray background
#define TEXT_COLOR      (Color){ 220, 220, 220, 255 }   // Light gray text
#define LINE_NUM_COLOR  (Color){ 100, 100, 100, 255 }   // Dim line numbers
#define CURSOR_COLOR    (Color){ 0, 255, 150, 255 }     // Cyan/green cursor
#define SEL_BG_COLOR    (Color){ 60, 80, 120, 255 }     // Selection background

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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("USAGE: %s <file-to-edit>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *filename = argv[1];

    // Create buffer
    DoublyLinkedList *buffer = create_list();
    if (!buffer) {
        fprintf(stderr, "Failed to create buffer\n");
        return EXIT_FAILURE;
    }

    if (file_load(filename, buffer) != EXIT_SUCCESS) {
        fprintf(stderr, "Error loading file: %s\n", filename);
        destroy_list(buffer);
        return EXIT_FAILURE;
    }

    buffer->current_line = buffer->head;
    buffer->cursor_col = 0;

    // Init Raylib
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(WIN_W, WIN_H, TextFormat("Text Editor - %s", filename));
    SetTargetFPS(FPS);
    SetExitKey(KEY_NULL);  // Disable ESC to close

    // Load TTF font
    // Font font = LoadFont("src/resources/Roboto/static/Roboto-Regular.ttf");
    // // LoadFontFromMemory(".ttf", fontData, sizeof(fontData), 32, NULL, 0);
    Font font = LoadFontFromMemory(".ttf", src_resources_Roboto_static_Roboto_Regular_ttf, src_resources_Roboto_static_Roboto_Regular_ttf_len, FONT_SIZE, NULL, 0);
    
    if (font.texture.id == 0) {
        // Fallback to default font if TTF not found
        font = GetFontDefault();
    }
    
    float fontSize = FONT_SIZE;
    int line_height = FONT_SIZE + LINE_SPACING;

    while (!WindowShouldClose()) {
        // Get actual window dimensions (changes when resized)
        int win_w = GetScreenWidth();
        int win_h = GetScreenHeight();
        
        float deltaTime = GetFrameTime();
        
        // --- Update cursor blink ---
        cursor_blink_timer += deltaTime;
        if (cursor_blink_timer >= CURSOR_BLINK_RATE) {
            cursor_blink_timer = 0.0f;
            cursor_visible = !cursor_visible;
        }
        
        // Reset blink on any input to make cursor immediately visible
        bool had_input = false;

        // --- Input ---
        if (IsKeyPressed(KEY_DOWN) || (IsKeyDown(KEY_DOWN) && IsKeyPressedRepeat(KEY_DOWN))) {
            move_cursor_down(buffer);
            update_scroll(buffer, line_height, win_h);
            had_input = true;
        }
        
        if (IsKeyPressed(KEY_UP) || (IsKeyDown(KEY_UP) && IsKeyPressedRepeat(KEY_UP))) {
            move_cursor_up(buffer);
            update_scroll(buffer, line_height, win_h);
            had_input = true;
        }
        
        if (IsKeyPressed(KEY_LEFT) || (IsKeyDown(KEY_LEFT) && IsKeyPressedRepeat(KEY_LEFT))) {
            move_cursor_left(buffer);
            had_input = true;
        }
        
        if (IsKeyPressed(KEY_RIGHT) || (IsKeyDown(KEY_RIGHT) && IsKeyPressedRepeat(KEY_RIGHT))) {
            move_cursor_right(buffer);
            had_input = true;
        }

        if (IsKeyPressed(KEY_BACKSPACE) || (IsKeyDown(KEY_BACKSPACE) && IsKeyPressedRepeat(KEY_BACKSPACE))) {
            delete_char(buffer);
            had_input = true;
        }
        
        if (IsKeyPressed(KEY_ENTER) || (IsKeyDown(KEY_ENTER) && IsKeyPressedRepeat(KEY_ENTER))) {
            insert_newline(buffer);
            update_scroll(buffer, line_height, win_h);
            had_input = true;
        }
        
        if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_S)) {
            if (save_file(buffer, filename) == EXIT_SUCCESS) {
                printf("File saved: %s\n", filename);
            } else {
                fprintf(stderr, "Error saving file: %s\n", filename);
            }
        }
        
        if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_Q)) {
            if (save_file(buffer, filename) == EXIT_SUCCESS) {
                printf("File saved: %s\n", filename);
            } else {
                fprintf(stderr, "Error saving file: %s\n", filename);
            }
            break;
        }
        
        if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_W)) {
            break;
        }
        
        // Mouse wheel scrolling
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            scroll_y -= (int)(wheel * 30);
            if (scroll_y < 0) scroll_y = 0;
        }
        

        // Character input
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key < 127) {  // Printable ASCII
                insert_char(buffer, (char)key);
                had_input = true;
            }
            key = GetCharPressed();
        }
        
        // Reset cursor blink on input
        if (had_input) {
            cursor_visible = true;
            cursor_blink_timer = 0.0f;
        }

        // --- Draw ---
        BeginDrawing();
        ClearBackground(BG_COLOR);
        
        int current_line_idx = get_line_index(buffer, buffer->current_line);
        int gutter_width = draw_line_numbers(buffer, font, fontSize, line_height, current_line_idx, win_h);
        draw_buffer(buffer, font, fontSize, line_height, gutter_width, win_w, win_h);
        
        // Draw status bar at bottom (using actual window height)
        int line_count = 0;
        Line *tmp = buffer->head;
        while (tmp) { line_count++; tmp = tmp->next; }
        
        char status[256];
        snprintf(status, sizeof(status), "Line %d/%d | Col %zu | %s", 
                 current_line_idx + 1, line_count, buffer->cursor_col, filename);
        
        int status_height = 25;
        int status_y = win_h - status_height;  // <-- Use actual window height here
        
        DrawRectangle(0, status_y, win_w, status_height, (Color){ 50, 50, 50, 255 });
        DrawLine(0, status_y, win_w, status_y, (Color){ 70, 70, 70, 255 });
        DrawTextEx(font, status, (Vector2){ 10, (float)(status_y + 4) }, 14, 1.0f, TEXT_COLOR);
        
        EndDrawing();
    }

    // Cleanup
    if (font.texture.id != 0) UnloadFont(font);
    CloseWindow();
    destroy_list(buffer);
    return EXIT_SUCCESS;
}