#include "ted.h"

// TODO: remove this file and move this function to buffer.c

void free_buffer(Buffer *buf) {
    all_lines_freedom(buf->cursor.y);
    free(buf->filename);
    free(buf->name);
}

