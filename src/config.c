#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "ex.h"
void config_load(Editor *e)
{
    const char *home = getenv("HOME");
    if (!home) return;
    char path[512];
    snprintf(path, sizeof(path), "%s/.viarc", home);
    FILE *f = fopen(path, "r");
    if (!f) return;
    char line[512];
    while (fgets(line, (int)sizeof(line), f)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';
        const char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '#' || *p == '"') continue; //neo: " is for comments just like vimscript
        ex_execute(e, p);
    }
    fclose(f);
}
