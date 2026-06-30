#include <string.h>
#include <curses.h>
#include "theme.h"
Theme via_theme;
static int parse_color(const char *name)
{
    if (!name) return -1;
    static const struct { const char *n; int c; } map[] = {
        {"black", COLOR_BLACK}, {"red", COLOR_RED},
        {"green", COLOR_GREEN}, {"yellow", COLOR_YELLOW},
        {"blue", COLOR_BLUE}, {"magenta", COLOR_MAGENTA},
        {"cyan", COLOR_CYAN}, {"white", COLOR_WHITE},
        {"default", -1}, {NULL, 0}
    };
    for (int i = 0; map[i].n; i++)
        if (strcmp(name, map[i].n) == 0) return map[i].c;
    return -1;
}

void theme_default(void)
{
    for (int i = 0; i < VIA_ELEMS; i++) {
        via_theme.fg[i] = COLOR_WHITE;
        via_theme.bg[i] = COLOR_BLACK;
        via_theme.attr[i] = 0;
    }
    via_theme.fg[VIA_STATUS] = COLOR_BLACK;
    via_theme.bg[VIA_STATUS] = COLOR_WHITE;
    via_theme.attr[VIA_STATUS] = A_BOLD;
    via_theme.fg[VIA_STATUS_INSERT] = COLOR_BLACK;
    via_theme.bg[VIA_STATUS_INSERT] = COLOR_CYAN;
    via_theme.attr[VIA_STATUS_INSERT] = A_BOLD;
    via_theme.fg[VIA_STATUS_VISUAL] = COLOR_BLACK;
    via_theme.bg[VIA_STATUS_VISUAL] = COLOR_YELLOW;
    via_theme.attr[VIA_STATUS_VISUAL] = A_BOLD;
    via_theme.fg[VIA_CMDLINE] = COLOR_WHITE;
    via_theme.bg[VIA_CMDLINE] = COLOR_BLACK;
    via_theme.attr[VIA_CMDLINE] = 0;
    via_theme.fg[VIA_LINENUM] = COLOR_YELLOW;
    via_theme.bg[VIA_LINENUM] = COLOR_BLACK;
    via_theme.attr[VIA_LINENUM] = 0;
    via_theme.fg[VIA_CURSOR_LINE] = COLOR_WHITE;
    via_theme.bg[VIA_CURSOR_LINE] = COLOR_BLUE;
    via_theme.attr[VIA_CURSOR_LINE] = 0;
    via_theme.fg[VIA_SELECTION] = COLOR_WHITE;
    via_theme.bg[VIA_SELECTION] = COLOR_BLUE;
    via_theme.attr[VIA_SELECTION] = 0;
    via_theme.fg[VIA_SEARCH] = COLOR_BLACK;
    via_theme.bg[VIA_SEARCH] = COLOR_YELLOW;
    via_theme.attr[VIA_SEARCH] = 0;
    via_theme.fg[VIA_EOL] = COLOR_WHITE;
    via_theme.bg[VIA_EOL] = COLOR_BLACK;
    via_theme.attr[VIA_EOL] = 0;
    via_theme.fg[VIA_KEYWORD] = COLOR_YELLOW;
    via_theme.bg[VIA_KEYWORD] = COLOR_BLACK;
    via_theme.attr[VIA_KEYWORD] = A_BOLD;
    via_theme.fg[VIA_STRING] = COLOR_GREEN;
    via_theme.bg[VIA_STRING] = COLOR_BLACK;
    via_theme.attr[VIA_STRING] = 0;
    via_theme.fg[VIA_COMMENT] = COLOR_CYAN;
    via_theme.bg[VIA_COMMENT] = COLOR_BLACK;
    via_theme.attr[VIA_COMMENT] = 0;
    via_theme.fg[VIA_NUMBER] = COLOR_MAGENTA;
    via_theme.bg[VIA_NUMBER] = COLOR_BLACK;
    via_theme.attr[VIA_NUMBER] = 0;
    via_theme.fg[VIA_TYPE] = COLOR_YELLOW;
    via_theme.bg[VIA_TYPE] = COLOR_BLACK;
    via_theme.attr[VIA_TYPE] = 0;
    via_theme.fg[VIA_PREPROC] = COLOR_MAGENTA;
    via_theme.bg[VIA_PREPROC] = COLOR_BLACK;
    via_theme.attr[VIA_PREPROC] = A_BOLD;
    via_theme.fg[VIA_OPERATOR] = COLOR_WHITE;
    via_theme.bg[VIA_OPERATOR] = COLOR_BLACK;
    via_theme.attr[VIA_OPERATOR] = 0;
}

void theme_init(void)
{
    if (!has_colors()) return;
    start_color();
    use_default_colors();
}

int theme_pair(int elem)
{
    if (elem < 0 || elem >= VIA_ELEMS) return 0;
    if (!has_colors()) return 0;
    short fg = via_theme.fg[elem];
    short bg = via_theme.bg[elem];
    init_pair(elem + 1, fg, bg);
    return COLOR_PAIR(elem + 1);
}

int theme_attr(int elem)
{
    if (elem < 0 || elem >= VIA_ELEMS) return 0;
    return via_theme.attr[elem];
}

int theme_set(int elem, const char *fg, const char *bg, const char *attr)
{
    if (elem < 0 || elem >= VIA_ELEMS) return -1;
    int f = parse_color(fg);
    int b = parse_color(bg);
    if (f < -1 || b < -1) return -1;
    via_theme.fg[elem] = (short)f;
    via_theme.bg[elem] = (short)b;
    via_theme.attr[elem] = 0;
    if (attr) {
        if (strstr(attr, "bold")) via_theme.attr[elem] |= A_BOLD;
        if (strstr(attr, "reverse")) via_theme.attr[elem] |= A_REVERSE;
        if (strstr(attr, "underline")) via_theme.attr[elem] |= A_UNDERLINE;
    }
    return 0;
}
