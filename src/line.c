#include "ted.h"

size_t get_ident(Line *ln) {
    size_t r = 0;
    for (size_t i = 0; ln->data[i] == ' '; i++)
        r++;
    return r;
}


void free_line(Line *line) {
    free(line->data);
    free(line);
}

Line *single_line(size_t cap) {
    Line ln = {
        malloc(cap * sizeof uchar32_t), cap, 0,
        NULL, NULL,
    };
    Line *r = malloc(sizeof ln);
    memcpy(r, &ln, sizeof ln);
    return r;
}

void line_reserve(size_t space, Line *line) {
    if (line->len + space > line->cap)
        while (line->len + space > line->cap)
            // + 1 in case line->cap is 0
            line->cap = 2 * line->cap + 1;
    
    line->data = realloc(line->data, line->cap * sizeof uchar32_t);
}

Line *prev_line(Line *line) {
    return line->prev ? line->prev : line;
}
Line *next_line(Line *line) {
    return line->next ? line->next : line;
}

void blank_line(Line *line) {
    line->len = 0;
}

void delete_line(Line *line) {
    if (line->prev)
        line->prev.next = line->next;
    if (line->next)
        line->next.prev = line->prev;

    free_line(line);
}

void append_line(Line *a, Line *line) {
    a->next = line->next;
    a->prev = line;

    line->next->prev = a;
    line->next = a;
}

void delete_character_from_line(size_t index, Line *line) {
    memmove(
        &line->data[index],
        &line->data[index + 1],
        (line->len - index - 1) * sizeof uchar32_t
    );
    line.len--;
}

void append_line_to_line(Line a, Line *line) {
    line_reserve(a.len, line);
    memcpy(
        &line->data[line->len],
        a.data,
        a.len * sizeof uchar32_t
    );
    line->len += a.len;
}