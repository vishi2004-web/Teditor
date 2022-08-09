#include "ted.h"

void die(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
    longjmp(end, 0);
}

char *home_path(const char *path) {
    char *ret = malloc(1000 * sizeof *ret);
    snprintf(ret, 1000, "%s/%s", getenv("HOME"), path);
    return ret;
}

/*
this function is similar to strtok_r, but tokenizes only on spaces
*/
char *split_spaces(char *str, char **save) {
    if (str == NULL) {
        if (*save == NULL)
            return NULL;
        str = *save;
    }

    if (*str == '\0') {
        *save = str;
        return NULL;
    }

    char *end = strchr(str, ' ');
    if (end == NULL) {
        *save = end;
        return str;
    }

    *end = '\0';
    *save = end + 1;
    while (isspace(**save)) (*save)++;// skip spaces
    return str;
}

char **split_str(const char *str, int *num_str) {
    char *strcp = malloc(strlen(str) + 1);
    char *origstrcp = strcp; // for free()
    strcpy(strcp, str);

    while (*strcp == ' ') strcp++; // Removes trailing spaces
    *num_str = 0;
    char **strs = NULL;

    while (1) {
        char *astr = malloc(1000);
        strcpy(astr, strcp);
        char *spc = strchr(strcp, ' ');

        if (spc) {
            astr[spc - strcp] = '\0';
            while (*spc == ' ') spc++;
        }

        strs = realloc(strs, ++(*num_str) * sizeof(*strs));

        strs[*num_str - 1] = astr;

        if (spc == NULL || !*spc)
            break;

        strcp = spc;
    }

    free(origstrcp);
    return strs;
}

int uchar32_cmp(const uchar32_t *s1, const char *s2, unsigned int stringlen) {
    for (unsigned int j = 0; j < stringlen; j++)
        if ((uchar32_t)s2[j] != s1[j])
            return 1; // Different character found

    return 0; // All characters equal
}

int uchar32_casecmp(const uchar32_t *s1, const char *s2, unsigned int stringlen) {
    for (unsigned int j = 0; j < stringlen; j++)
        if ((uchar32_t)tolower(s2[j]) != s1[j] && (uchar32_t)toupper(s2[j]) != s1[j])
            return 1; // Different character found

    return 0; // All characters equal
}

int uchar32_sub(const uchar32_t *hs, const char *sub, unsigned int hslen, unsigned int sublen) {
    for (unsigned int i = 0; i < hslen; i++)
        if (!uchar32_cmp(&hs[i], sub, sublen))
            return i;// Substring found, return index of the match

    return -1; // No substring found
}

char *bufn(int a) {
    const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char *s = malloc(100);
    char *p = s;

    do {
        *(p++) = letters[a % sizeof(letters)];
        a = a / sizeof(letters);
    } while (a >= sizeof(letters));
    *p = '\0';

    return s;
}

int number_length(unsigned long num, int base) {
    int i;
    for (i = 1; num; i++) {
        num /= base;
        i++;
    };

    return i;
}
