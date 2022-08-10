#include "ted.h"

Buffer empty_buffer() {
    Line *fst_line = single_line(0);

    static int buffer_count = -1;
    buffer_count++;

    Buffer b = {
        false,
        false,
        true,
        0,
        {0, 0, fst_line},
        {0, fst_line},
        bufn(buffer_count),
        NULL,
    };

    return b;
}

void delete_line_from_buffer(Line *line, Buffer *buf) {
    if (buf->cursor.y == line)
        buf->cursor.y = line->next ? line->next : line->prev;

    if (buf->scroll.y == line)
        buf->scroll.y = buf->cursor.y;

    delete_line(line);
}

void delete_selected_line(Buffer *buf) {
    if (buf->cursor.y->next || buf->cursor.y->prev)
        delete_line_from_buffer(buf->cursor.y, buf);
    else
        blank_line(buf->cursor.y);
}

void delete_character_before_selected(Buffer *buf) {
    if (buf->cursor.x > 0)
        delete_character_from_line(buf->cursor.x - 1, buf->cursor.y);
    else if (buf->cursor.y->prev) {
        append_line_to_line(*buf->cursor.y, buf->cursor.y->prev);
        delete_line_from_buffer(buf->cursor.y, buf);
    } else {
        // BEEP
    }
}

void break_selected_line(Buffer *buf) {
    Line *fst = buf->cursor.y;
    Line *snd = single_line(fst->len - buf->cursor.x);

    memcpy(snd->data, &fst->data[buf->cursor.x], snd->cap * sizeof(uchar32_t));

    fst->len = buf->cursor.x;
    
    append_line(snd, fst);
}

void type_character(uchar32_t c, Buffer *buf) {
    line_insert(c, buf->cursor.x, buf->cursor.y);
    buf->cursor.x++;
}



// Should be called after incrementing the cursor position
void cursor_to_valid_position(Buffer *buf) {
    /* Check if cursor is inside the borders */
    if (buf->cursor.x > buf->cursor.y->len)
        buf->cursor.x = buf->cursor.y->len;
}

bool modify(Buffer *buf) {
    if (buf->read_only)
        message("Can't modify a read-only file.");
    else
        buf->modified = 1;

    return !buf->read_only;
}


