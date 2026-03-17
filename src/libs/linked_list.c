#include "linked_list.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
 #include <stdio.h>

Line *create_line(const char *text) {
    if(!text) text = "";
    
    Line *line = malloc(sizeof(Line));
    if (line == NULL) return NULL;
    
    line->len = strlen(text);
    line->capacity = line->len + 16;
    line->text = malloc(line->capacity);
    if(!line->text) {
        free(line);
        return NULL;
    }
    
    strcpy(line->text, text);
    
    line->prev = NULL;
    line->next = NULL;
    
    return line;
}

DoublyLinkedList *create_list(void){
    DoublyLinkedList *list = malloc(sizeof(DoublyLinkedList));
    if (list == NULL) {
        return NULL;
    }
    list->head = NULL;
    list->tail = NULL;
    list->cursor_col = 0;
    return list;
}

void destroy_list(DoublyLinkedList *list) {
    if (list == NULL) {
        return;
    }
    Line *current = list->head;
    while (current != NULL) {
        Line *next = current->next;
        free(current->text);
        free(current);
        current = next;
    }
    free(list);
}


void insert_line_after(DoublyLinkedList *list, Line *line, const char *text){
    if(!list) return;
    
    Line *new_line = create_line(text);
    if(!new_line) return;
    
    // if no line provided, insert at head
    if(!line) {
        new_line->next = list->head;
        if(list->head){
            list->head->prev = new_line;
        }
        list->head = new_line;
        
        if(!list->tail){
            list->tail = new_line;
        }
        
        return;
    }
    
    new_line->next = line->next;
    new_line->prev = line;
    
    if(line->next) 
        line->next->prev = new_line;
    else
        list->tail = new_line;
    
    line->next = new_line;
    
}



// insert a new line **before** a given line
void insert_line_before(DoublyLinkedList *list, Line *line, const char *text) {
    if (!list) return;

    Line *new_line = create_line(text);
    if (!new_line) return;

    if (!line) { // insert at tail if line is NULL
        new_line->prev = list->tail;
        if (list->tail)
            list->tail->next = new_line;
        list->tail = new_line;
        if (!list->head)
            list->head = new_line;
        return;
    }

    new_line->next = line;
    new_line->prev = line->prev;

    if (line->prev)
        line->prev->next = new_line;
    else
        list->head = new_line;

    line->prev = new_line;
}

// append a new line at the **end** of the list
void append_line(DoublyLinkedList *list, const char *text) {
    if (!list) return;

    Line *new_line = create_line(text);
    if (!new_line) return;

    if (!list->head) {
        list->head = new_line;
        list->tail = new_line;
        return;
    }

    new_line->prev = list->tail;
    list->tail->next = new_line;
    list->tail = new_line;
}

void delete_line(DoublyLinkedList *list, Line *line) {
    if (!list || !line) return;

    // Adjust previous and next links
    if (line->prev)
        line->prev->next = line->next;
    else
        list->head = line->next;  // deleted line was head

    if (line->next)
        line->next->prev = line->prev;
    else
        list->tail = line->prev;  // deleted line was tail

    // Move cursor if it was pointing to this line
    if (list->current_line == line) {
        if (line->next)
            list->current_line = line->next;
        else
            list->current_line = line->prev;
        list->cursor_col = 0; // reset cursor column
    }

    // Free memory
    free(line->text);
    free(line);
}



// Move cursor up by one line
void move_cursor_up(DoublyLinkedList *list) {
    if (!list || !list->current_line) return;

    if (list->current_line->prev) {
        list->current_line = list->current_line->prev;

        // Clamp cursor_col to the new line length
        if (list->cursor_col > list->current_line->len)
            list->cursor_col = list->current_line->len;
    }
}

// Move cursor down by one line
void move_cursor_down(DoublyLinkedList *list) {
    if (!list || !list->current_line) return;

    if (list->current_line->next) {
        list->current_line = list->current_line->next;

        // Clamp cursor_col to the new line length
        if (list->cursor_col > list->current_line->len)
            list->cursor_col = list->current_line->len;
    }
}

// Move cursor left by one character
void move_cursor_left(DoublyLinkedList *list) {
    if (!list || !list->current_line) return;

    if (list->cursor_col > 0) {
        list->cursor_col--;
    } else if (list->current_line->prev) {
        // move to end of previous line
        list->current_line = list->current_line->prev;
        list->cursor_col = list->current_line->len;
    }
}

// Move cursor right by one character
void move_cursor_right(DoublyLinkedList *list) {
    if (!list || !list->current_line) return;

    if (list->cursor_col < list->current_line->len) {
        list->cursor_col++;
    } else if (list->current_line->next) {
        // move to start of next line
        list->current_line = list->current_line->next;
        list->cursor_col = 0;
    }
}


void insert_char(DoublyLinkedList *list, char c) {
    if (!list || !list->current_line) return;

    Line *line = list->current_line;

    // Resize if necessary
    if (line->len + 1 >= line->capacity) {
        line->capacity *= 2;
        char *new_text = realloc(line->text, line->capacity);
        if (!new_text) return; // allocation failed
        line->text = new_text;
    }

    // Shift characters to the right
    memmove(line->text + list->cursor_col + 1,
            line->text + list->cursor_col,
            line->len - list->cursor_col + 1); // +1 for null terminator

    // Insert the character
    line->text[list->cursor_col] = c;
    line->len++;
    list->cursor_col++;
}


void delete_char(DoublyLinkedList *list) {
    if (!list || !list->current_line) return;

    Line *line = list->current_line;

    if (list->cursor_col > 0) {
        // Shift characters left
        memmove(line->text + list->cursor_col - 1,
                line->text + list->cursor_col,
                line->len - list->cursor_col + 1); // +1 for null terminator
        line->len--;
        list->cursor_col--;
    } else if (line->prev) {
        // Merge with previous line
        Line *prev = line->prev;
        size_t new_len = prev->len + line->len;
        
        // Resize previous line if needed
        if (prev->capacity < new_len + 1) {
            prev->capacity = new_len + 16;
            prev->text = realloc(prev->text, prev->capacity);
        }

        // Append current line to previous
        memcpy(prev->text + prev->len, line->text, line->len + 1); // +1 for null terminator
        prev->len = new_len;

        // Remove current line
        prev->next = line->next;
        if (line->next)
            line->next->prev = prev;
        else
            list->tail = prev;


        list->current_line = prev;
        list->cursor_col = prev->len - line->len; // cursor at join point
        
        free(line->text);
        free(line);
    }
}


void insert_newline(DoublyLinkedList *list) {
    if (!list || !list->current_line) return;

    Line *line = list->current_line;

    // Create new line with text after cursor
    const char *rest = line->text + list->cursor_col;
    Line *new_line = create_line(rest);
    if (!new_line) return;

    // Shrink current line
    line->len = list->cursor_col;
    line->text[line->len] = '\0';

    // Insert new line after current
    new_line->next = line->next;
    new_line->prev = line;
    if (line->next)
        line->next->prev = new_line;
    else
        list->tail = new_line;
    line->next = new_line;

    // Move cursor to new line
    list->current_line = new_line;
    list->cursor_col = 0;
}




bool parse_file(const char *filename, DoublyLinkedList *buffer) {
    if (!buffer) return false;

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file");
        return false;
    }

    char line[1024]; // temporary buffer for each line
    while (fgets(line, sizeof(line), fp)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        append_line(buffer, line);
    }

    fclose(fp);
    return true;
}

int file_load(const char *filename, DoublyLinkedList *buffer) {
    if (access(filename, F_OK) == 0) {
        return parse_file(filename, buffer) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create file");
        return EXIT_FAILURE;
    }
    fputs("\n", fp);
    fclose(fp); // close the newly created file

    return parse_file(filename, buffer) ? EXIT_SUCCESS : EXIT_FAILURE;
}

void print_buffer(DoublyLinkedList *buffer) {
    printf("\n\n\n========\n");
    for (Line *cur = buffer->head; cur; cur = cur->next) {
        printf("%s\n", cur->text);
    }
    printf("========\n\n");
    
}

int save_file(DoublyLinkedList *buffer, char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to open file for writing");
        return EXIT_FAILURE;
    }

    for (Line *cur = buffer->head; cur; cur = cur->next) {
        fprintf(fp, "%s\n", cur->text);
    }

    fclose(fp);
    return EXIT_SUCCESS;
}