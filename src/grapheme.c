#include "ted.h"

/* Some functions built on top of suckless' libgrapheme */

Grapheme get_next_grapheme(char **str, size_t len) {
    Grapheme grapheme = {
        grapheme_next_character_break_utf8(*str, len),
        *str,
    };

    *str += grapheme.sz;

    return grapheme;
}

size_t grapheme_width(Grapheme g) {
    if (3 == g.sz && 0 == memcmp("の", g.dt, g.sz)) {
        return 2;
    }

    return 1;
}

size_t wi_to_gi(size_t si, char *s) {
    for (size_t i = 0;; i++) {
        Grapheme g = get_next_grapheme(&s, SIZE_MAX);

        if (0 == g.sz)
            return i;

        size_t width = grapheme_width(g);

        if (width > si)
            return i;
        si -= width;
    }
}

size_t gi_to_wi(size_t gi, char *s) {
    size_t r = 0;
    for (size_t i = 0; i < gi; i++) {
        Grapheme g = get_next_grapheme(&s, SIZE_MAX);

        if (g.sz == 0)
            break;

        r += grapheme_width(g);
    }
    return r;
}
