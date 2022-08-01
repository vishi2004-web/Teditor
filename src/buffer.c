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

void clear_line_existence(Line *line, Buffer *buf) {
    if (buf->cursor.y == line)
        buf->cursor.y = line.next ? line.next : line.prev;

    if (buf->textscroll.y == line)
        buf->textscroll.y = buf->cursor.y;
}

void delete_selected_line(Buffer *buf) {
    if (buf->cursor.y.next || buf->cursor.y.prev) {
        delete_line(buf->cursor.y);
        clear_line_existence(buf->cursor.y, buf);
    } else {
        blank_line(buf->cursor.y);
    }
}

void delete_character_before_selected(Buffer *buf) {
    if (buf->cursor.x > 0)
        delete_character_from_line(buf->cursor.x - 1, buf->cursor.y);
    else if (buf->cursor.y.prev) {
        append_line_to_line(*buf->cursor.y, buf->cursor.y.prev);
        delete_line(buf->cursor.y);
        clear_line_existence(buf->cursor.y, buf);
    } else {
        // BEEP
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
