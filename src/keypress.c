#include "ted.h"

void expand_line(size_t at, int x, Buffer *buf) {
    if (buf->lines[at].len <= buf->lines[at].length + x) {
        while (buf->lines[at].len <= buf->lines[at].length + x)
            buf->lines[at].len *= 2;

        buf->lines[at].data = realloc(
            buf->lines[buf->cursor.y].data,
            buf->lines[buf->cursor.y].len
        );
    }
}

// TODO: remake this function w/ an easier to understand declaration
void new_line(size_t at, int x, Buffer *buf) {
    buf->lines[at].len = READ_BLOCKSIZE;
    buf->lines[at].data = malloc(buf->lines[at].len);
    buf->lines[at].length = 0;

    expand_line(at, buf->lines[at - 1].length - x + 1, buf);

    memcpy(
        buf->lines[at].data,
        &buf->lines[at - 1].data[x],
        buf->lines[at - 1].length - x
    );

    buf->lines[at].length += buf->lines[at - 1].length - x;
    buf->lines[at].data[buf->lines[at].length] = '\0';

    buf->lines[at - 1].length = x;
    buf->lines[at - 1].data[buf->lines[at - 1].length] = '\0';
}

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
        // Decrements `y` if it is greater than 0
        buf->cursor.y -= buf->cursor.y > 0;

        buf->cursor.x_grapheme = buf->cursor.last_x_grapheme;
        calculate_cursor_x(buf);

        break;
    case KEY_DOWN:
    case ctrl('n'):
        // Increments `y` if it doesn't gets greater or equal than `num_lines`
        buf->cursor.y += buf->cursor.y + 1 < buf->num_lines;

        buf->cursor.x_grapheme = buf->cursor.last_x_grapheme;
        calculate_cursor_x(buf);

        break;
    case KEY_LEFT:
    case ctrl('b'):
        buf->cursor.x_grapheme -= buf->cursor.x_grapheme > 0;
        calculate_cursor_x(buf);
        buf->cursor.last_x_grapheme = buf->cursor.x_grapheme;

        break;
    case KEY_RIGHT:
    case ctrl('f'):
        // No need to check anything since `calculate_cursor_x` will truncate it
        // if it overflows
        buf->cursor.x_grapheme++;
        calculate_cursor_x(buf);
        buf->cursor.last_x_grapheme = buf->cursor.x_grapheme;

        break;
    case KEY_HOME:
    case ctrl('a'):
        buf->cursor.x_grapheme = 0;
        calculate_cursor_x(buf);
        buf->cursor.last_x_grapheme = buf->cursor.x_grapheme;

        break;
    case KEY_END:
    case ctrl('e'):
        buf->cursor.x_grapheme = SIZE_MAX;
        calculate_cursor_x(buf);
        buf->cursor.last_x_grapheme = buf->cursor.x_grapheme;

        break;
    case ctrl('s'):
        if (!buf->read_only)
            savefile(buf);
        break;
    case '\t':
        if (config.use_spaces == 1) {
            for (int i = 0; i < config.tablen; i++)
                process_keypress(' ', n);
            return false;
        } // else, it will pass though and be added to the buffer
        break;
    case ctrl('g'):
        if (config_dialog(n))
            return true;
        break;
    case ctrl('q'):
        if (parse_command(
            buf->read_only ? "read-only 0" : "read-only 1",
            n
        ))
            return true;
        break;
    // TODO: reimplement these
    /*case KEY_PPAGE:
    {
        unsigned int ccy = buf->cursor.y;
        for (unsigned int i = 0; i < (unsigned int)(ccy % config.lines + config.lines) && buf->cursor.y > 0; i++)
            buf->cursor.y--;
        cursor_in_valid_position(buf);
        break;
    } case KEY_NPAGE:
    {
        unsigned int ccy = buf->cursor.y;
        for (unsigned int i = 0; i < (unsigned int)(config.lines - (ccy % config.lines) - 1 + config.lines) && buf->cursor.y < buf->num_lines - 1; i++)
            buf->cursor.y++;
        cursor_in_valid_position(buf);
        break;
    } */case KEY_MOUSE:
    {
        MEVENT event;
        if (getmouse(&event) == OK)
            process_mouse_event(event, n);

        break;
    // TODO: reimplement these tootoo
    } /*case 0x209:
    {
        if (modify(buf)) {
            if (buf->num_lines > 1) {
                free(buf->lines[buf->cursor.y].data);
                memmove(
                    &buf->lines[buf->cursor.y],
                    &buf->lines[buf->cursor.y + 1],
                    (buf->num_lines - buf->cursor.y - 1) * sizeof(*buf->lines)
                );
                buf->lines = realloc(buf->lines, --buf->num_lines * sizeof(*buf->lines));
            } else {
                buf->lines[buf->cursor.y].data[0] = '\0';
                buf->lines[buf->cursor.y].length = 0;
            }
            cursor_in_valid_position(buf);
        }
        break;
    } case ctrl('w'):
    {
        bool passed_spaces = 0;
        while (buf->cursor.x > 0 && (!strchr(config.word_separators, buf->lines[buf->cursor.y].data[buf->cursor.x - 1]) || !passed_spaces)) {
            if (!remove_char(buf->cursor.x - 1, buf->cursor.y, buf))
                break;
            process_keypress(KEY_LEFT, n);
            if (buf->cursor.x > 0 && !strchr(config.word_separators, buf->lines[buf->cursor.y].data[buf->cursor.x - 1]))
                passed_spaces = 1;
        }
        break;
    }*/ case ctrl('o'):
    {
        char *d = prompt("open: ", buf->filename);
        if (d)
            open_file(d, n);
        break;
    // TODO:
    } /*case CTRL_KEY_LEFT:
    {
        char passed_spaces = 0;
        while (buf->cursor.x > 0) {
            process_keypress(KEY_LEFT, n);
            if (!strchr(config.word_separators, buf->lines[buf->cursor.y].data[buf->cursor.x]))
                passed_spaces = 1;
            if (strchr(config.word_separators, buf->lines[buf->cursor.y].data[buf->cursor.x]) && passed_spaces) {
                process_keypress(KEY_RIGHT, n);
                break;
            }
        }
        break;
    }
    case CTRL_KEY_RIGHT:
    {
        char passed_spaces = 0;
        while (buf->lines[buf->cursor.y].data[buf->cursor.x] != '\0' && !(strchr(config.word_separators, buf->lines[buf->cursor.y].data[buf->cursor.x]) && passed_spaces)) {
            if (!strchr(config.word_separators, buf->lines[buf->cursor.y].data[buf->cursor.x]))
                passed_spaces = 1;
            process_keypress(KEY_RIGHT, n);
        }
        break;
    } */case KEY_BACKSPACE: case KEY_DC: case 127:
    {
        if (modify(buf)) {
            if (buf->cursor.x > 0) {
                if (buf->cursor.x <= buf->lines[buf->cursor.y].ident)
                    buf->lines[buf->cursor.y].ident--;
                
                if (remove_char(buf->cursor.x - 1, buf->cursor.y, buf))
                    process_keypress(KEY_LEFT, n);
            } else if (buf->cursor.y > 0) {
                Line del_line = &buf->lines[buf->cursor.y];

                memmove(
                    &buf->lines[buf->cursor.y],
                    &buf->lines[buf->cursor.y + 1],
                    (buf->num_lines - buf->cursor.y - 1) * sizeof(Line)
                );
                buf->num_lines--;

                buf->cursor.y--;
                calculate_cursor_x(buf);

                // TODO: check if process_keypress(KEY_RIGHT) is really not needed

                expand_line(buf->cursor.y, del_line.length, buf);

                Line *to_append = buf->lines[buf->cursor.y];

                memmove(
                    &to_append->data[to_append->length],
                    del_line.data,
                    del_line.length,
                );

                to_append->length += del_line.length;
                // TODO: check that expand_line adds space for the null byte
                to_append->data[to_append->length] = '\0';


                to_append->ident = 0;
                for (size_t i = 0; ' ' == to_append.data[i]; i++)
                    to_append->ident++;


                free(del_line.data);
            }
        }
        break;
    } case '\n': case KEY_ENTER: case '\r':
    {
        if (modify(buf)) {
            buf->lines = realloc(buf->lines, ++buf->num_lines * sizeof(Line));
            memmove(
                &buf->lines[buf->cursor.y + 2],
                &buf->lines[buf->cursor.y + 1],
                (buf->num_lines - buf->cursor.y - 2) * sizeof(Line),
            )

            size_t lcx = buf->cursor.x;
            buf->cursor.y++;
            buf->cursor.x_grapheme = 0;
            calculate_cursor_x(buf);
            buf->cursor.last_x_grapheme = buf->cursor.x_grapheme;

            new_line(buf->cursor.y, lcx, buf);

            if (config.autotab) {
                size_t ident = buf->lines[buf->cursor.y - 1].ident;
                buf->lines[buf->cursor.y].ident = ident;
                buf->lines[buf->cursor.y].len += ident;
                buf->lines[buf->cursor.y].data = realloc(
                    buf->lines[buf->cursor.y].data,
                    buf->lines[buf->cursor.x].len,
                );
                
                memmove(
                    &buf->lines[buf->cursor.y].data[ident],
                    buf->lines[buf->cursor.y].data,
                    buf->lines[buf->cursor.y].length + 1,
                );

                // TODO: since we're back to bytes we can use memset
                for (size_t i = 0; i < ident; i++)
                    buf->lines[buf->cursor.y].data[i] = ' ';
                buf->lines[buf->cursor.y].length += ident;

                for (size_t i = 0; i < ident; i++)
                    process_keypress(KEY_RIGHT, n);
            } else {
                buf->lines[buf->cursor.y]->ident = 0;
                for (size_t i = 0; ' ' == buf->lines[buf->cursor.y].data[i]; i++)
                    buf->lines[buf->cursor.y]->ident++;
            }
        }
        break;
    }
    }

    

    if (isprint(c) || c == '\t' || (c >= 0xC0 && c <= 0xDF) || (c >= 0xE0 && c <= 0xEF) || (c >= 0xF0 && c <= 0xF7)) {

        if (modify(buf)) {
            if (c == ' ' && buf->cursor.x <= buf->lines[buf->cursor.y].ident)
                buf->lines[buf->cursor.y].ident++;

            unsigned char ucs[4] = {c, 0, 0, 0};
            int len = 1;

            if ((c >= 0xC2 && c <= 0xDF) || (c >= 0xE0 && c <= 0xEF) || (c >= 0xF0 && c <= 0xF4)) {
                ucs[1] = getch(), len++;
            }
            if ((c >= 0xE0 && c <= 0xEF) || (c >= 0xF0 && c <= 0xF4)) {
                ucs[2] = getch(), len++;
            }
            if (c >= 0xF0 && c <= 0xF4) {
                ucs[3] = getch(), len++;
            }

            if (validate_utf8(ucs)) {
                uchar32_t ec = c;
                for (int i = 1, off = 8; i < len; i++, off += 8)
                    ec += ucs[i] << off;

                if (add_char(buf->cursor.x, buf->cursor.y, ec, buf))
                    process_keypress(KEY_RIGHT, n);
            } else {
                for (int i = 0; i < len; i++) {
                    if (add_char(buf->cursor.x, buf->cursor.y, substitute_char, buf))
                        process_keypress(KEY_RIGHT, n);
                    else
                        break;
                }
            }
        }
    }
    return false;
}

