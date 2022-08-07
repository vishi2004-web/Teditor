#ifndef TED_HEADER
#define TED_HEADER

#define _POSIX_C_SOURCE 1

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <setjmp.h>
#include <limits.h>

#define READ_BLOCKSIZE 100
#define ctrl(x) ((x) & 0x1f)

#define CTRL_KEY_RIGHT 0x232
#define CTRL_KEY_LEFT  0x223

#define NUM_PAIRS 6

#define IN_RANGE(x, min, max)  ((x) >= (min)) && ((x) <= (max))
#define OUT_RANGE(x, min, max) ((x) < (min)) || ((x) > (max))

// timeout for input in ncurses (in milliseconds)
#define INPUT_TIMEOUT 5

#define USE(x) (void)(x)

#define MSG_SZ 512

// Types

typedef uint32_t uchar32_t;

typedef struct Line {
    uchar32_t *data;
    size_t cap;
    size_t len;

    struct Line *next;
    struct Line *prev;
} Line;

typedef struct {
    size_t x;
    size_t last_x;
    Line *y;
} Cursor;

typedef struct {
    size_t x;
    Line *y;
} TextScroll;

typedef struct {
    bool modified;
    bool read_only;
    bool can_write;
    unsigned char line_break_type; // 0: LF  1: CRLF
    Cursor cursor;
    TextScroll scroll;
    char *name;
    char *filename;
} Buffer;

typedef struct {
    bool strict_utf8; // high/low surrogates will be replaced (for now leave it always set)
    int lines;
    size_t tablen;
    bool use_spaces;
    bool autotab;
} GlobalCfg;

typedef struct {
    const char *command;
    const char *hint;
} Hints;

typedef struct Node {
    Buffer data;
    struct Node *next;
    struct Node *prev;
} Node;


// message_and_prompt.c
char *prompt(const char *msgtmp, char *def);
char *prompt_hints(const char *msgtmp, char *def, char *base, Hints *hints);
void message(char *msg);

// ted.c
void setcolor(int c);

// config_dialog.c
bool config_dialog(Node **n);
int run_command(char **words, int words_len, Node **n);
bool parse_command(char *command, Node **n);

// fileio.c
bool savefile(Buffer *buf);
Buffer read_lines(FILE *fp, char *filename, bool read_only);
unsigned char detect_linebreak(FILE *fp);
void open_file(char *fname, Node **n);
bool can_write(char *fname);

// display.c
void display_menu(const char *message, const char *shadow, Node *n);
void display_buffer(Buffer buf, int len_line_number);

// free.c
void free_buffer(Buffer *buf);

// keypress.c
void expand_line(unsigned int at, int x, Buffer *buf);
void new_line(unsigned int at, int x, Buffer *buf);
bool process_keypress(int c, Node **n);

// mouse.c
bool process_mouse_event(MEVENT ev, Node **n);

// utf8.c
unsigned char utf8_size(unsigned char c);
uchar32_t utf8_to_utf32(unsigned char ucs[4]);
void utf8ReadFile(unsigned char uc, uchar32_t *out, FILE *fp_);
int utf8ToMultibyte(uchar32_t c, unsigned char *out, bool validate);
bool validate_utf8(unsigned char *ucs);

// utils.c
void die(const char *s);
char *home_path(const char *path);
char *split_spaces(char *str, char **save);
char **split_str(const char *str, int *num_str);
int calculate_len_line_number(Buffer buf);
int uchar32_cmp(const uchar32_t *s1, const char *s2, unsigned int stringlen);
int uchar32_casecmp(const uchar32_t *s1, const char *s2, unsigned int stringlen);
int uchar32_sub(const uchar32_t *hs, const char *sub, unsigned int hslen, unsigned int sublen);
char *bufn(int a);

// scroll.c
void calculate_scroll(Buffer *buf, int len_line_number);

// buffer_list.c
Node *allocate_node(Node n);
void deallocate_node(Node *n);
Node *single_buffer(Buffer b);
void buffer_add_next(Node *n, Buffer b);
void buffer_add_prev(Node *n, Buffer b);
void buffer_close(Node *n);
void free_buffer_list(Node *n);

// buffer.c
Buffer empty_buffer();
void clear_line_existence(Line *line, Buffer *buf);
void delete_selected_line(Buffer *buf);
void delete_character_before_selected(Buffer *buf);
void break_selected_line(Buffer *buf);


void cursor_to_valid_position(Buffer *buf);
bool modify(Buffer *buf);

// line.c
size_t get_ident(Line *ln);
void free_line(Line *line);
Line *single_line(size_t cap);
void line_reserve(size_t space, Line *line);
Line *prev_line(Line *line);
Line *next_line(Line *line);
void blank_line(Line *line);
void delete_line(Line *line);
void append_line(Line *a, Line *line);
void delete_character_from_line(size_t index, Line *line);
void append_line_to_line(Line a, Line *line);

extern GlobalCfg config;
extern char *menu_message;
extern const uchar32_t substitute_char;
extern const char *substitute_string;
extern jmp_buf end;

#endif

