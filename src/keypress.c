#include "ted.h"

// TODO: rewrite everything here from scratch
// TODO: separate the actual input processing with the buffer modification,
//       since it is confusing to have both in the same file
// FIXME: when the buffer is reallocated, it gets READ_BLOCKSIZE plus space, but
//        it may be more efficient to have the capacity multiplied by 2 instead
//        of this, just like most implementations of a dynamic array


bool process_keypress(int c, Node **n) {
    Buffer *buf = &(*n)->data;
    if (c != ERR)
        message("");

    switch (c) {
    case ctrl('c'):
        return parse_command("close", n);
    case ctrl('z'):
        return parse_command("prev", n);
    case ctrl('x'):
        return parse_command("next", n);
    case KEY_UP:
    case ctrl('p'):
        buf->cursor.x = buf->cursor.last_x;
        cursor_to_valid_position(buf);
        buf->cursor.y = prev_line(buf->cursor.y);
        break;
    case KEY_DOWN:
    case ctrl('n'):
        buf->cursor.x = buf->cursor.last_x;
        cursor_to_valid_position(buf);
        buf->cursor.y = next_line(buf->cursor.y);
        break;
    case KEY_LEFT:
    case ctrl('b'):
        buf->cursor.x -= buf->cursor.x > 0;
        buf->cursor.last_x = buf->cursor.x;
        break;
    case KEY_RIGHT:
    case ctrl('f'):
        buf->cursor.x++;
        cursor_to_valid_position(buf);
        buf->cursor.last_x = buf->cursor.x;
        break;
    case KEY_HOME:
    case ctrl('a'):
        buf->cursor.x = 0;
        buf->cursor.last_x = buf->cursor.x;
        break;
    case KEY_END:
    case ctrl('e'):
        buf->cursor.x = buf->cursor.y->len;
        buf->cursor.last_x = buf->cursor.x;
        break;
    case ctrl('s'):
        if (!buf->read_only)
            savefile(buf);
        break;
    case '\t': // TAB
        if (config.use_spaces == 1) {
            for (size_t i = 0; i < config.tablen; i++)
                process_keypress(' ', n);
            return false;
        } // else, it will pass though and be added to the buffer
        break;
    case ctrl('g'):
        return config_dialog(n);
    case ctrl('q'):
        return parse_command(
            buf->read_only ? "read-only 0" : "read-only 1",
            n
        );
    case KEY_PPAGE: { // PAGE_UP
        /*
        buf->cursor.y -= buf->cursor.y % config.lines;
        buf->cursor.y -= buf->cursor.y >= config.lines
            ? config.lines
            : buf->cursor.y;
            */
        // TODO: reimplement
        break;
    } case KEY_NPAGE: { // PAGE_DOWN
        /*buf->cursor.y += 2 * config.lines - buf->cursor.y % config.lines - 1;
        cursor_in_valid_position(buf);*/
        // TODO: reimplement
        break;
    } case KEY_MOUSE: { // Any mouse event
        MEVENT event;
        if (getmouse(&event) == OK)
            process_mouse_event(event, n);
        break;
    } case 0x209: { // Control + DELETE
        if (modify(buf))
            delete_selected_line(buf);
        break;
    } case ctrl('w'): {
        if (modify(buf)) {
            // FIXME: reimplement Ctrl-W
        }
        break;
    } case ctrl('o'): {
        char *d = prompt("open: ", buf->filename);
        if (d)
            open_file(d, n);
        break;
    } case CTRL_KEY_LEFT: { // Control + KEY_LEFT
        // FIXME: reimplement this
        break;
    }
    case CTRL_KEY_RIGHT: { // Control + KEY_RIGHT
        // FIXME: reimplement this
        break;
    } case KEY_BACKSPACE: case KEY_DC: case 127: { // BACKSPACE; DELETE; DELETE
        if (modify(buf))
            delete_character_before_selected(buf);
        break;
    } case '\n': case KEY_ENTER: case '\r': { // NEWLINE; ENTER; CARRIAGE RETURN
        if (modify(buf))
            break_selected_line(buf);
        // TODO: implement autotab
        break;
    }
    }

    if (isprint(c) || c == '\t' || (c >= 0xC0 && c <= 0xDF) || (c >= 0xE0 && c <= 0xEF) || (c >= 0xF0 && c <= 0xF7)) {
        if (modify(buf)) {
            int len = utf8_size(c);
            unsigned char ucs[4] = {c, 0, 0, 0};

            for (int i = 1; i < len; i++)
                ucs[i] = getch();

            if (validate_utf8(ucs))
                type_character(utf8_to_utf32(ucs, len), buf);
            else
                message("Invalid input, is your terminal UTF-8?");
        }
    }
    return false;
}

