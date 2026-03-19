#ifndef GLOBALS_H
#define GLOBALS_H


#define WIN_W 800
#define WIN_H 600
#define FPS 60
#define LINE_SPACING 4
#define FONT_SIZE 18
#define PADDING_X 20
#define PADDING_Y 20
#define CURSOR_BLINK_RATE 0.53f // Cursor blink interval in seconds

// color scheme 
#define BG_COLOR        (Color){ 30, 30, 30, 255 }      // Dark gray background
#define TEXT_COLOR      (Color){ 220, 220, 220, 255 }   // Light gray text
#define LINE_NUM_COLOR  (Color){ 100, 100, 100, 255 }   // Dim line numbers
#define CURSOR_COLOR    (Color){ 0, 255, 150, 255 }     // Cyan/green cursor
#define SEL_BG_COLOR    (Color){ 60, 80, 120, 255 }     // Selection background

// global scroll offset.
extern int scroll_y;

// cursor blink state
extern float cursor_blink_timer;
extern bool cursor_visible;

#endif // GLOBALS_H