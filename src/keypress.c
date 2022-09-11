#include "ted.h"

// garants that the capacity is (x + 1) bytes greater than the length
void expand_line(Line *ln, size_t x) {
    if (ln->cap <= ln->length + x + 1) {
        while (ln->cap <= ln->length + x + 1)
            ln->cap *= 2;

        ln->data = realloc(ln->data, ln->cap);
    }
}


bool process_keypress(int c, Node **n) {
    Buffer *buf = &(*n)->data;

    if (c == ERR)
        return false;

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

        recalc_cur(buf);

        break;
    case KEY_DOWN:
    case ctrl('n'):
        // Increments `y` if it doesn't gets greater or equal than `num_lines`
        buf->cursor.y += buf->cursor.y + 1 < buf->num_lines;

        recalc_cur(buf);

        break;
    case KEY_LEFT:
    case ctrl('b'): {
        char *s = buf->lines[buf->cursor.y].data;

        size_t x_grapheme = wi_to_gi(buf->cursor.x_width, s);

        if (x_grapheme > 0) {
            buf->cursor.x_width = gi_to_wi(x_grapheme - 1, s);
            truncate_cur(buf);
            buf->cursor.lx_width = buf->cursor.x_width;
        }

        break;
    }
    case KEY_RIGHT:
    case ctrl('f'): {
        char *s = buf->lines[buf->cursor.y].data + buf->cursor.x_bytes;
        Grapheme g = get_next_grapheme(&s, SIZE_MAX);
        size_t gw = grapheme_width(g);

        buf->cursor.x_width += gw;
        truncate_cur(buf);
        buf->cursor.lx_width = buf->cursor.x_width;

        break;
    }
    case KEY_HOME:
    case ctrl('a'):
        buf->cursor.x_width = 0;
        truncate_cur(buf);
        buf->cursor.lx_width = buf->cursor.x_width;

        break;
    case KEY_END:
    case ctrl('e'):
        buf->cursor.x_width = SIZE_MAX;
        truncate_cur(buf);
        buf->cursor.lx_width = buf->cursor.x_width;

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
            // FIXME: remove_char now needs modify()
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
            if (buf->cursor.x_bytes > 0) {
                process_keypress(KEY_LEFT, n);
                remove_char(buf->cursor.x_bytes, &buf->lines[buf->cursor.y]);
            } else if (buf->cursor.y > 0) {
                Line del_line = buf->lines[buf->cursor.y];

                memmove(
                    &buf->lines[buf->cursor.y],
                    &buf->lines[buf->cursor.y + 1],
                    (buf->num_lines - buf->cursor.y - 1) * sizeof(Line)
                );
                buf->num_lines--;

                buf->cursor.y--;
                buf->cursor.x_width = SIZE_MAX;
                truncate_cur(buf);
                buf->cursor.lx_width = buf->cursor.x_width;

                expand_line(&buf->lines[buf->cursor.y], del_line.length);

                Line *to_append = &buf->lines[buf->cursor.y];

                memmove(
                    &to_append->data[to_append->length],
                    del_line.data,
                    del_line.length
                );

                to_append->length += del_line.length;
                to_append->data[to_append->length] = '\0';

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
                (buf->num_lines - buf->cursor.y - 2) * sizeof(Line)
            );

            buf->lines[buf->cursor.y + 1] = blank_line();

            Line *current = &buf->lines[buf->cursor.y];
            Line *new = &buf->lines[buf->cursor.y + 1];

            if (config.autotab) {
                size_t ident_sz = get_ident_sz(current->data);
                
                expand_line(new, ident_sz);

                memcpy(new->data, current->data, ident_sz);
                new->length = ident_sz;
                new->data[new->length] = '\0';
            }


            size_t cur_x = buf->cursor.x_bytes;

            buf->cursor.y++;
            buf->cursor.x_width = SIZE_MAX;
            truncate_cur(buf);
            buf->cursor.lx_width = buf->cursor.x_width;

            expand_line(new, current->length - cur_x);

            memcpy(
                &new->data[new->length],
                &current->data[cur_x],
                // + 1 so that it also copies the null byte
                current->length + 1 - cur_x
            );

            new->length += current->length - cur_x;

            current->length = cur_x;
            current->data[current->length] = '\0';
        }

        break;
    }
    }

    // TODO: move this all to `default`

    char cc[4];
    cc[0] = c;

    uint32_t codepoint;
    size_t r = grapheme_decode_utf8(cc, 1, &codepoint);

    if (GRAPHEME_INVALID_CODEPOINT == codepoint || r != 1) {
        if (r > 1) {
            for (size_t i = 1; i < r; i++)
                cc[i] = getch();

            size_t newr = grapheme_decode_utf8(cc, r, &codepoint);
            if (GRAPHEME_INVALID_CODEPOINT == codepoint || newr != r)
                return false;
        } else {
            // TODO: maybe I can print a message?
            return false;
        }
    }


    if (r > 1 || isprint(c) || '\t' == c) {
        if (modify(buf)) {
            Grapheme g = {r, cc};

            add_char(g, buf->cursor.x_bytes, &buf->lines[buf->cursor.y]);
            process_keypress(KEY_RIGHT, n);
        }
    }
    

    return false;
}

