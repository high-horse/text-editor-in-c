#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <raylib.h>
#include "libs/linked_list.h"

#define WIN_W 800
#define WIN_H 600
#define FPS 60
#define LINE_SPACING 2
#define FONT_SIZE 20

// Global scroll offset
int scroll_y = 0;

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

// Measure width of first 'len' characters
int MeasureTextLen(const char *text, size_t len, int fontSize) {
    if (!text) return 0;
    char tmp[len + 1];
    strncpy(tmp, text, len);
    tmp[len] = '\0';
    return MeasureText(tmp, fontSize);
}

// Draw linked-list buffer with cursor and scrolling
void draw_buffer(DoublyLinkedList *buffer, Font font) {
    if (!buffer) return;

    int y = 20 - scroll_y;
    Line *cur = buffer->head;

    while (cur) {
        Color color = RAYWHITE;
        if (cur == buffer->current_line) {
            // highlight current line
            DrawRectangle(15, y - 2, WIN_W - 30, FONT_SIZE + LINE_SPACING, LIGHTGRAY);
            color = RED;
        }
        DrawTextEx(font, cur->text, (Vector2){20, (float)y}, 16, 2, color);
        y += FONT_SIZE + LINE_SPACING;
        cur = cur->next;
    }

    // Draw cursor
    if (buffer->current_line) {
        int cursor_x = 20 + MeasureTextLen(buffer->current_line->text, buffer->cursor_col, font.baseSize);
        int cursor_y = get_line_index(buffer, buffer->current_line) * (FONT_SIZE + LINE_SPACING) - scroll_y + 20;
        DrawRectangle(cursor_x, cursor_y, 2, FONT_SIZE, RED);
    }
}

// Update scroll so current line is visible
void update_scroll(DoublyLinkedList *buffer) {
    if (!buffer->current_line) return;
    int line_y = get_line_index(buffer, buffer->current_line) * (FONT_SIZE + LINE_SPACING);
    if (line_y - scroll_y < 0) scroll_y = line_y;
    else if (line_y - scroll_y > WIN_H - FONT_SIZE - LINE_SPACING)
        scroll_y = line_y - (WIN_H - FONT_SIZE - LINE_SPACING);
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
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WIN_W, WIN_H, "Text Editor");
    SetTargetFPS(FPS);

    // Load TTF font
    Font font = LoadFont("src/resources/Roboto/static/Roboto-Regular.ttf");
    if (font.texture.id == 0) {
        printf("Failed to load font!\n");
        CloseWindow();
        destroy_list(buffer);
        return EXIT_FAILURE;
    }
    
    // Repeat timing
    // float key_timer_down = 0.0f;
    // float key_timer_up = 0.0f;
    // const float KEY_DELAY = 0.2f;   // initial delay before repeat (seconds)
    // const float KEY_REPEAT = 0.05f; // repeat interval (seconds)

    while (!WindowShouldClose()) {
        // --- Input ---
        if (IsKeyDown(KEY_DOWN)) {
            move_cursor_down(buffer);
            update_scroll(buffer);
        } 
        
        if (IsKeyDown(KEY_UP)) {
            move_cursor_up(buffer);
            update_scroll(buffer);
        }
        
        if (IsKeyDown(KEY_LEFT)) move_cursor_left(buffer);
        
        if (IsKeyDown(KEY_RIGHT)) move_cursor_right(buffer);

        if (IsKeyDown(KEY_BACKSPACE)) delete_char(buffer);
        
        if (IsKeyDown(KEY_ENTER)) insert_newline(buffer);
        
        int key = GetCharPressed();
        while (key > 0) {
            if (key == 8) delete_char(buffer); // backspace
            else if (key == 13) insert_newline(buffer); // enter
            else insert_char(buffer, (char)key);
            key = GetCharPressed();
        }

        // --- Draw ---
        BeginDrawing();
        ClearBackground(BLACK);
        draw_buffer(buffer, font);
        EndDrawing();
    }

    // Cleanup
    UnloadFont(font);
    CloseWindow();
    destroy_list(buffer);
    return EXIT_SUCCESS;
}