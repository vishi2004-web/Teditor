size_t get_ident(Line *ln) {
    size_t r = 0;
    for (size_t i = 0; ln.data[i] == ' '; i++)
        r++;
    return r;
}
