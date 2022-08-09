/*
TODO: use wcwidth and replace code points for grapheme clusters in order to have
      a better Unicode handling
*/

#include "ted.h"
#include <curses.h>
#include <unistd.h>

char *menu_message = "";

GlobalCfg config = {
    1, 0, 4, 1, 1,
};

jmp_buf end;

void create_config(char dir[PATH_MAX]) {
    strcpy(dir, getenv("HOME"));
    strcat(dir, "/.config");

    struct stat st = {0};

    if (stat(dir, &st) == -1)
        mkdir(dir, 0777); // FIXME: maybe this is not the best permission
    
    strcat(dir, "/ted");

    if (stat(dir, &st) == -1)
        mkdir(dir, 0777);
}

int main(int argc, char **argv) {
    int is_input_pipe = !isatty(STDIN_FILENO);
    if (is_input_pipe) {
        // TODO: read pipe contents to buffer before closing it
        close(STDIN_FILENO);
        open("/dev/tty", O_RDONLY);
    }

    Node *buf = NULL;
    if (argc < 2) {
        char buffer[PATH_MAX];
        create_config(buffer);
        strcat(buffer, "/buffer");

        char *filename = malloc(strlen(buffer) + 1);
        strcpy(filename, buffer);

        FILE *fp = fopen(filename, "r");
        buf = single_buffer(read_file_into_buffer(fp, filename));
        if (fp)
            fclose(fp);
    } else {
        for (int i = 1; i < argc; i++) {
            char *filename = malloc(PATH_MAX + 1);
            size_t len = 0;

            if (*argv[i] == '/') {
                // Absolute file path
                len = strlen(argv[i]);
                memcpy(filename, argv[i], len + 1);
            } else {
                // Relative file path

                // Write the directory path into filename
                if (getcwd(filename, PATH_MAX) != NULL) {
                    len = strlen(filename);
                    len += snprintf(
                        filename + len,
                        PATH_MAX - len,
                        "/%s",
                        argv[i]
                    );
                } else
                    die("Could not get cwd, try an absolute file path");

                // Now we have a absolute filename
            }

            char *smaller_filename = malloc(len + 1);
            memcpy(smaller_filename, filename, len + 1);
            free(filename);
            filename = smaller_filename;

            FILE *fp = fopen(filename, "r");
            Buffer b = read_file_into_buffer(fp, filename);
            if (i == 1)
                buf = single_buffer(b);
            else
                buffer_add_prev(buf, b);

            if (fp)
                fclose(fp);
        }
    }

    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);
    mouseinterval(1);
    curs_set(0);
    timeout(INPUT_TIMEOUT);


    config.lines = LINES - 1;
    int last_LINES = LINES;
    int last_COLS = COLS;

    int val = setjmp(end);

    if (!val) {
        while (1) {
            if (last_LINES != LINES || last_COLS != COLS) {
                last_LINES = LINES;
                last_COLS = COLS;
                config.lines = LINES - 1;
                cursor_to_valid_position(&buf->data);
            }

            int len_line_number = number_length(config.lines, 10);

            calculate_scroll(&buf->data, len_line_number);

            display_buffer(buf->data, len_line_number);
            display_menu(menu_message, NULL, buf);
            refresh();

            int c = getch();
            if (process_keypress(c, &buf))
                break;
        }
    }

    free_buffer_list(buf);

    endwin();
    return 0;
}
