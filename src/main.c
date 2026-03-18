#include <stdbool.h>
#include <time.h>
#include "libs/font_data.h"
#include "libs/raylib_paint.h"

// Global scroll offset
int scroll_y = 0;

// Cursor blink state
float cursor_blink_timer = 0.0f;
bool cursor_visible = true;


// Add this helper near your other utilities
int calculate_gutter_width(DoublyLinkedList *buffer, Font font, float fontSize) {
    int line_count = 0;
    Line *cur = buffer->head;
    while (cur) { line_count++; cur = cur->next; }
    
    char num_str[16];
    snprintf(num_str, sizeof(num_str), "%d", line_count);
    int gutter_width = MeasureTextEx(font, num_str, fontSize, 1.0f).x + 30; // 20 + 10 padding
    
    return gutter_width;
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
        int gutter_width = calculate_gutter_width(buffer, font, fontSize);
        
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
        
        handle_mouse_click(buffer, font, fontSize, line_height, gutter_width, win_h);

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
        // int gutter_width = draw_line_numbers(buffer, font, fontSize, line_height, current_line_idx, win_h);
        draw_line_numbers(buffer, font, fontSize, line_height, current_line_idx, win_h);
        
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