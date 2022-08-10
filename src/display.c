#include "ted.h"

// Buffer* is used instead of Buffer for being able to pass NULL
void display_menu(const char *message, const char *shadow, Node *n) {
    const Buffer *buf = &n->data;
    int x, y;
    getyx(stdscr, y, x);

    move(config.lines, 0);
    for (size_t i = 0; i < COLS; i++)
        addch(' ');

    move(config.lines, 0);
    if (!*message && n) {
        printw("I:%u %s", get_ident(buf->cursor.y), buf->filename);

        char b[500];
        int len = snprintf(
            b, 500, "%c%c%c %s | %s<-%s->%s",
            buf->modified ? '!' : '.',
            buf->read_only ? 'o' : 'c',
            buf->can_write ? 'W' : 'R',
            buf->line_break_type == 0 ? "LF" : "CRLF",
            n->prev->data.name,
            buf->name,
            n->next->data.name
        );

        mvprintw(config.lines, COLS - len, "%s", b);

    } else if (shadow != NULL) {
        printw("%s", message);
        getyx(stdscr, y, x); //save message x and y

        attron(A_BOLD);
        printw("%s", shadow);

        attroff(A_BOLD);
    } else
        printw("%s", message);

    move(y, x);
}

void display_buffer(Buffer buf, int len_line_number) {
    Line *line = buf.scroll.y;

    size_t i;
    for (i = 0; i < config.lines; i++) {
        move(i, 0);
        
        if (line) {
            if (line == buf.cursor.y)
                attron(A_BOLD);

            printw("%*d ", len_line_number, i + 1);

            attroff(A_BOLD);

            size_t size = 0;

            for (size_t j = 0; size < COLS - len_line_number - 1; j++) {
                if (j + buf.scroll.x == buf.cursor.x && line == buf.cursor.y)
                    attron(A_REVERSE);

                if (buf.scroll.x + j >= line->len) {
                    addch(' ');
                } else {
                    fprintf(stderr, "%lx %lx %lx %lx\n", line, line->data, buf.scroll.x, j);
                    uchar32_t el = line->data[buf.scroll.x + j];

                    // TODO: handle full-width unicode characters

                    if (el == '\t') {
                        for (size_t k = 0; k < config.tablen; k++)
                            addch(' ');
                        size += config.tablen - 1;
                    } else {
                        unsigned char b[4];
                        int len = utf8ToMultibyte(el, b, 1);

                        if (len == -1) {
                            b[0] = substitute_string[0];
                            b[1] = substitute_string[1];
                            b[2] = substitute_string[2];
                            len = 3;
                        }
                        printw("%.*s", len, b);
                    }
                }

                attroff(A_REVERSE);
                size++;
            }


            line = line->next;
        } else {
            for (int j = 0; j < len_line_number - 1; j++)
                addch(' ');
            addch('~');
            for (int j = 0; j < COLS - len_line_number; j++)
                addch(' ');
        }
    }
}
