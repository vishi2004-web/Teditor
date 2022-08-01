Buffer empty_buffer() {
    Line *fst_line = single_line();

    static int buffer_count = -1;

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

void delete_selected_line(Buffer *buf) {
    if (buf->cursor.y.next || buf->cursor.y.prev) {
        Line *deleted = buf->cursor.y;
        buf->cursor.y = deleted.next ? deleted.next : deleted.prev;

        if (buf->textscroll.y == deleted)
            buf->textscroll.y = buf->cursor.y;
        
        delete_line(deleted);
    } else {
        blank_line(buf->cursor.y);
    }
}

// Should be called after incrementing the cursor position
void cursor_to_valid_position(Buffer *buf) {
    /* Check if cursor is inside the borders */
    if (buf->cursor.x > buf->cursor.y->length)
        buf->cursor.x = buf->cursor.y->length;
}

bool modify(Buffer *buf) {
    if (buf->read_only)
        message("Can't modify a read-only file.");
    else
        buf->modified = 1;

    return !buf->read_only;
}
