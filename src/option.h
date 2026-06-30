#ifndef VIA_OPTION_H
#define VIA_OPTION_H
#include <stdbool.h>
typedef struct {
    int tabstop;
    bool number;
    bool list;
    int scrolloff;
    int shiftwidth;
    bool hlsearch;
    bool incsearch;
    bool smartcase;
    bool ignorecase;
    bool syntax;
} Options;
extern Options via_opts;
void opt_init(void);
int opt_set(const char *name, const char *value);
#endif
