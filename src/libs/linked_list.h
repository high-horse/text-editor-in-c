#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


typedef struct Line{
    char *text;
    size_t len;
    size_t capacity;
    struct Line *prev;
    struct Line *next;
} Line;

typedef struct {
    Line *head;
    Line *tail;
    Line *current_line;
    size_t cursor_col;
} DoublyLinkedList;

// create node
Line *create_line(const char *text);

// list management
DoublyLinkedList *create_list(void);
void destroy_list(DoublyLinkedList *list);

// insert
void insert_line_after(DoublyLinkedList *list, Line *line, const char *text);
void insert_line_before(DoublyLinkedList *list, Line *line, const char *text);
void append_line(DoublyLinkedList *list, const char *text);

// deletion
void delete_line(DoublyLinkedList *list, Line *line);

// cursor movement
void move_cursor_up(DoublyLinkedList *list);
void move_cursor_down(DoublyLinkedList *list);
void move_cursor_left(DoublyLinkedList *list);
void move_cursor_right(DoublyLinkedList *list);


// line editing
void insert_char(DoublyLinkedList *list, char c);
void delete_char(DoublyLinkedList *list);
void insert_newline(DoublyLinkedList *list);

// file operation
int file_load(const char *filename, DoublyLinkedList *buffer);
void print_buffer(DoublyLinkedList *buffer);
int save_file(DoublyLinkedList *buffer, char *filename);
// int file_load(DoublyLinkedList *list, const char *filename);
// int file_save(DoublyLinkedList *list, const char *filename);

// utility
size_t get_line_count(DoublyLinkedList *list);


#endif // LINKED_LIST_H