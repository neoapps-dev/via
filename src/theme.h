#ifndef VIA_THEME_H
#define VIA_THEME_H
#define VIA_ELEMS 12
typedef struct {
    short fg[VIA_ELEMS];
    short bg[VIA_ELEMS];
    int attr[VIA_ELEMS];
} Theme;
enum {
    VIA_NORMAL,
    VIA_INSERT,
    VIA_VISUAL,
    VIA_STATUS,
    VIA_STATUS_INSERT,
    VIA_STATUS_VISUAL,
    VIA_CMDLINE,
    VIA_LINENUM,
    VIA_CURSOR_LINE,
    VIA_SELECTION,
    VIA_SEARCH,
    VIA_EOL,
};

extern Theme via_theme;
void theme_init(void);
void theme_default(void);
int theme_pair(int elem);
int theme_attr(int elem);
int theme_set(int elem, const char *fg, const char *bg, const char *attr);
#endif
