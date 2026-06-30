#ifndef VIA_SYNTAX_H
#define VIA_SYNTAX_H
#include <stdbool.h>
#define SYN_SPANS_MAX 256
typedef struct {
    int offset;
    int length;
    int elem;
} SynSpan;
typedef struct {
    bool in_block;
} SynState;
const char *syn_detect(const char *filename);
int syn_highlight(const char *line, int len, const char *ft, SynSpan *spans, int max, SynState *state);
#endif
