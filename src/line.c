size_t get_ident(Line *ln) {
    size_t r = 0;
    for (size_t i = 0; ln.data[i] == ' '; i++)
        r++;
    return r;
}



void free_line(Line *line) {
    free(line->data);
    free(line);
}

Line *single_line() {
    Line ln = {
        NULL, 0, 0,
        NULL, NULL,
    };
    Line *r = malloc(sizeof ln);
    memcpy(r, &ln, sizeof ln);
    return r;
}

Line *prev_line(Line *line) {
    return line.prev ? line.prev : line;
}
Line *next_line(Line *line) {
    return line.next ? line.next : line;
}

void blank_line(Line *line) {
    line.len = 0;
}

void delete_line(Line *line) {
    if (line.prev)
        line.prev.next = line.next;
    if (line.next)
        line.next.prev = line.prev;

    free_line(line);
}
