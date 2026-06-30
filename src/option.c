#include <string.h>
#include <stdlib.h>
#include "option.h"
Options via_opts;
void opt_init(void)
{
    via_opts.tabstop = 8;
    via_opts.number = false;
    via_opts.list = false;
    via_opts.scrolloff = 0;
    via_opts.shiftwidth = 8;
    via_opts.hlsearch = true;
    via_opts.incsearch = false;
    via_opts.smartcase = true;
    via_opts.ignorecase = false;
    via_opts.syntax = true;
}

static int opt_set_int(const char *name, const char *value)
{
    char *end;
    long n = strtol(value, &end, 10);
    if (*end) return -1;
    if (strcmp(name, "tabstop") == 0) { via_opts.tabstop = (int)n; return 0; }
    if (strcmp(name, "shiftwidth") == 0) { via_opts.shiftwidth = (int)n; return 0; }
    if (strcmp(name, "scrolloff") == 0) { via_opts.scrolloff = (int)n; return 0; }
    return -1;
}

static int opt_set_bool(const char *name, const char *value)
{
    bool v;
    if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) v = true;
    else if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0) v = false;
    else return -1;
    if (strcmp(name, "number") == 0) { via_opts.number = v; return 0; }
    if (strcmp(name, "list") == 0) { via_opts.list = v; return 0; }
    if (strcmp(name, "hlsearch") == 0) { via_opts.hlsearch = v; return 0; }
    if (strcmp(name, "incsearch") == 0) { via_opts.incsearch = v; return 0; }
    if (strcmp(name, "smartcase") == 0) { via_opts.smartcase = v; return 0; }
    if (strcmp(name, "ignorecase") == 0) { via_opts.ignorecase = v; return 0; }
    if (strcmp(name, "syntax") == 0) { via_opts.syntax = v; return 0; }
    return -1;
}

int opt_set(const char *name, const char *value)
{
    const char *nums[] = {"tabstop", "shiftwidth", "scrolloff", NULL};
    const char *bools[] = {"number", "list", "hlsearch", "incsearch", "smartcase", "ignorecase", "syntax", NULL};
    for (int i = 0; nums[i]; i++)
        if (strcmp(name, nums[i]) == 0) return opt_set_int(name, value);
    for (int i = 0; bools[i]; i++)
        if (strcmp(name, bools[i]) == 0) return opt_set_bool(name, value);
    return -1;
}
