
// Should be called after incrementing the cursor position
void cursor_to_valid_position(Buffer *buf) {
    /* Check if cursor is inside the borders */
    // y
    if (buf->cursor.y >= buf->num_lines)
        buf->cursor.y = buf->num_lines - 1;
    // x
    if (buf->cursor.x > buf->lines[buf->cursor.y].length)
        buf->cursor.x = buf->lines[buf->cursor.y].length;
}

bool modify(Buffer *buf) {
    if (buf->read_only)
        message("Can't modify a read-only file.");
    else
        buf->modified = 1;

    return !buf->read_only;
}
