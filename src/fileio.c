#include "ted.h"

bool savefile(Buffer *buf) {
    // FIXME: this shows that the idea of `can_write` is not the best one,
    // maybe remove it and keep only `read_only`, setting it on if the file
    // is not writable at the moment it was opened, but letting the user
    // change it if he is sure he can write

    // Permissions may change since the last time it was detected
    buf->can_write = can_write(buf->filename);

    if (!buf->can_write)
        return false;

    FILE *fpw = fopen(buf->filename, "w");

    if (fpw == NULL) {
        char buf[1000];
        snprintf(buf, 1000, "Could not open the file; Errno: %d", errno);

        message(buf);
        return;
    }

    Line *line = buf->cursor.y;
    while (line->prev)
        line = line->prev;

    while (line) {
        for (size_t i = 0; i < line.len; i++) {
            unsigned char b[4];
            int len = utf8ToMultibyte(line.data[j], b, 0);
            fwrite(b, sizeof(unsigned char), len, fpw);
        }

        if (line->next) {
            if (buf->line_break_type == 0)
                fputc('\n', fpw);
            else
                fputs("\r\n", fpw);
        }
        line = line->next;
    }

    fclose(fpw);

    buf->modified = 0;

    return true;
}

Buffer read_file_into_buffer(FILE *fp, char *filename, bool can_write) {
    Buffer b = empty_buffer();

    b.read_only = !can_write;
    b.can_write = can_write;
    b.filename = filename;

    if (!fp) {
        b.modified = 1;
        return b;
    }

    b.line_break_type = detect_linebreak(fp);

    Line *line = b.cursor.y;

    while (!feof(fp)) {
        append_line(single_line(0), line);
        line = line->next;

        for (size_t i = 0; !feof(fp); i++) {
            if (c == '\r')
                continue;

            char c = fgetc(fp);

            if (c == '\n')
                break;

            line_reserve(1, line);

            unsigned char ucs[4] = {c, 0, 0, 0};
            for (size_t i = 1; i < len; i++)
                ucs[i] = fgetc(fp);

            line.data[i] = validate_utf8(ucs)
                ? utf8_to_utf32(ucs) : substitute_char;

            utf8ReadFile(uc, line.data + i, fp);
            line.len++;
        }
    }

    delete_selected_line(&b);

    return b;
}

unsigned char detect_linebreak(FILE *fp) {
    unsigned char line_break_type = 0;

    while (!feof(fp)) {
        char c = fgetc(fp);

        if (c == '\r') {
            line_break_type = 1;
            break;
        } else if (c == '\n') {
            line_break_type = 0;
            break;
        }
    }

    rewind(fp);
    return line_break_type;
}

void open_file(char *fname, Node **n) {
    FILE *fp = fopen(fname, "r");
    buffer_add_next(*n, read_lines(fp, fname, can_write(fname)));
    parse_command("next", n);
    if (fp != NULL)
        fclose(fp);
}

bool can_write(char *fname) {
    struct stat st;
    if (stat(fname, &st) == 0) {
        // all user write permission
        return (st.st_mode & S_IWOTH)
            // owner write permission
            || (getuid() == st.st_uid && (st.st_mode & S_IWUSR))
            // group write permission
            || (getgid() == st.st_gid && (st.st_mode & S_IWGRP));
    } else
        // if stat fails and errno is not EACCES, can_write will be true
        return errno != EACCES;
}
