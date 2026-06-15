#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "buffer.h"
Buffer *buf_new(void)
{
    Buffer *b = malloc(sizeof(Buffer));
    b->cap = 16;
    b->count = 1;
    b->lines = malloc(b->cap * sizeof(char *));
    b->lens = malloc(b->cap * sizeof(int));
    b->lines[0] = malloc(1);
    b->lines[0][0] = '\0';
    b->lens[0] = 0;
    return b;
}

void buf_free(Buffer *b)
{
    for (int i = 0; i < b->count; i++)
        free(b->lines[i]);
    free(b->lines);
    free(b->lens);
    free(b);
}

int buf_load(Buffer *b, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    for (int i = 0; i < b->count; i++)
        free(b->lines[i]);
    b->count = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t n;
    while ((n = getline(&line, &len, f)) != -1) {
        if (n > 0 && line[n - 1] == '\n') line[n - 1] = '\0';
        if (n > 1 && line[n - 2] == '\r') line[n - 2] = '\0';
        if (b->count >= b->cap) {
            b->cap *= 2;
            b->lines = realloc(b->lines, b->cap * sizeof(char *));
            b->lens = realloc(b->lens, b->cap * sizeof(int));
        }
        b->lines[b->count] = strdup(line);
        b->lens[b->count] = (int)strlen(b->lines[b->count]);
        b->count++;
    }
    free(line);
    fclose(f);
    if (b->count == 0) {
        b->lines[0] = malloc(1); b->lines[0][0] = '\0';
        b->lens[0] = 0;
        b->count = 1;
    }
    return 0;
}

int buf_save(Buffer *b, const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    for (int i = 0; i < b->count; i++) {
        fwrite(b->lines[i], 1, (size_t)b->lens[i], f);
        if (i < b->count - 1) fputc('\n', f);
    }
    fclose(f);
    return 0;
}

void buf_insert_char(Buffer *b, int row, int col, char c)
{
    if (row < 0 || row >= b->count) return;
    b->lines[row] = realloc(b->lines[row], (size_t)(b->lens[row] + 2));
    memmove(b->lines[row] + col + 1, b->lines[row] + col, (size_t)(b->lens[row] - col + 1));
    b->lines[row][col] = c;
    b->lens[row]++;
}

void buf_delete_char(Buffer *b, int row, int col)
{
    if (row < 0 || row >= b->count) return;
    if (col < b->lens[row]) {
        memmove(b->lines[row] + col, b->lines[row] + col + 1, (size_t)(b->lens[row] - col));
        b->lens[row]--;
    } else if (row < b->count - 1) {
        int nl = b->lens[row] + b->lens[row + 1];
        b->lines[row] = realloc(b->lines[row], (size_t)(nl + 1));
        memcpy(b->lines[row] + b->lens[row], b->lines[row + 1], (size_t)(b->lens[row + 1] + 1));
        b->lens[row] = nl;
        free(b->lines[row + 1]);
        memmove(b->lines + row + 1, b->lines + row + 2, (size_t)(b->count - row - 2) * sizeof(char *));
        memmove(b->lens + row + 1, b->lens + row + 2, (size_t)(b->count - row - 2) * sizeof(int));
        b->count--;
    }
}

void buf_insert_line(Buffer *b, int row)
{
    if (row < 0 || row > b->count) return;
    if (b->count >= b->cap) {
        b->cap *= 2;
        b->lines = realloc(b->lines, (size_t)b->cap * sizeof(char *));
        b->lens = realloc(b->lens, (size_t)b->cap * sizeof(int));
    }
    memmove(b->lines + row + 1, b->lines + row, (size_t)(b->count - row) * sizeof(char *));
    memmove(b->lens + row + 1, b->lens + row, (size_t)(b->count - row) * sizeof(int));
    b->lines[row] = malloc(1);
    b->lines[row][0] = '\0';
    b->lens[row] = 0;
    b->count++;
}

void buf_delete_line(Buffer *b, int row)
{
    if (row < 0 || row >= b->count) return;
    free(b->lines[row]);
    memmove(b->lines + row, b->lines + row + 1, (size_t)(b->count - row - 1) * sizeof(char *));
    memmove(b->lens + row, b->lens + row + 1, (size_t)(b->count - row - 1) * sizeof(int));
    b->count--;
}

void buf_join(Buffer *b, int row)
{
    if (row < 0 || row >= b->count - 1) return;
    int nl = b->lens[row] + b->lens[row + 1];
    b->lines[row] = realloc(b->lines[row], (size_t)(nl + 1));
    memcpy(b->lines[row] + b->lens[row], b->lines[row + 1], (size_t)(b->lens[row + 1] + 1));
    b->lens[row] = nl;
    free(b->lines[row + 1]);
    memmove(b->lines + row + 1, b->lines + row + 2, (size_t)(b->count - row - 2) * sizeof(char *));
    memmove(b->lens + row + 1, b->lens + row + 2, (size_t)(b->count - row - 2) * sizeof(int));
    b->count--;
}

void buf_split(Buffer *b, int row, int col)
{
    if (row < 0 || row >= b->count) return;
    if (b->count >= b->cap) {
        b->cap *= 2;
        b->lines = realloc(b->lines, (size_t)b->cap * sizeof(char *));
        b->lens = realloc(b->lens, (size_t)b->cap * sizeof(int));
    }
    int tail_len = b->lens[row] - col;
    char *tail = malloc((size_t)(tail_len + 1));
    memcpy(tail, b->lines[row] + col, (size_t)tail_len);
    tail[tail_len] = '\0';
    b->lines[row][col] = '\0';
    b->lens[row] = col;
    memmove(b->lines + row + 1, b->lines + row, (size_t)(b->count - row) * sizeof(char *));
    memmove(b->lens + row + 1, b->lens + row, (size_t)(b->count - row) * sizeof(int));
    b->lines[row + 1] = tail;
    b->lens[row + 1] = tail_len;
    b->count++;
}

void buf_clear(Buffer *b)
{
    for (int i = 0; i < b->count; i++)
        free(b->lines[i]);
    b->count = 1;
    if (b->cap < 16) {
        b->cap = 16;
        b->lines = realloc(b->lines, (size_t)b->cap * sizeof(char *));
        b->lens = realloc(b->lens, (size_t)b->cap * sizeof(int));
    }
    b->lines[0] = malloc(1);
    b->lines[0][0] = '\0';
    b->lens[0] = 0;
}

char *buf_line(Buffer *b, int row)
{
    if (row < 0 || row >= b->count) return NULL;
    return b->lines[row];
}

int buf_len(Buffer *b, int row)
{
    if (row < 0 || row >= b->count) return 0;
    return b->lens[row];
}

int buf_count(Buffer *b)
{
    return b->count;
}

void buf_insert_str(Buffer *b, int row, int col, const char *s, int len)
{
    if (row < 0 || row >= b->count) return;
    b->lines[row] = realloc(b->lines[row], (size_t)(b->lens[row] + len + 1));
    memmove(b->lines[row] + col + len, b->lines[row] + col, (size_t)(b->lens[row] - col + 1));
    memcpy(b->lines[row] + col, s, (size_t)len);
    b->lens[row] += len;
}

void buf_delete_range(Buffer *b, int r1, int c1, int r2, int c2)
{
    if (r1 < 0 || r1 >= b->count) return;
    if (r2 < 0 || r2 >= b->count) return;
    if (r1 > r2 || (r1 == r2 && c1 >= c2)) return;
    if (r1 == r2) {
        memmove(b->lines[r1] + c1, b->lines[r1] + c2, (size_t)(b->lens[r1] - c2 + 1));
        b->lens[r1] -= (c2 - c1);
        return;
    }
    int new_len = c1 + b->lens[r2] - c2;
    for (int r = r1 + 1; r < r2; r++)
        new_len += b->lens[r];
    char *new = malloc((size_t)(new_len + 1));
    int pos = 0;
    memcpy(new + pos, b->lines[r1], (size_t)c1); pos += c1;
    for (int r = r1 + 1; r < r2; r++) {
        memcpy(new + pos, b->lines[r], (size_t)b->lens[r]); pos += b->lens[r];
    }
    memcpy(new + pos, b->lines[r2] + c2, (size_t)(b->lens[r2] - c2)); pos += b->lens[r2] - c2;
    new[pos] = '\0';
    free(b->lines[r1]);
    b->lines[r1] = new;
    b->lens[r1] = new_len;
    int remove = r2 - r1;
    for (int i = r1 + 1; i + remove < b->count; i++) {
        free(b->lines[i]);
        b->lines[i] = b->lines[i + remove];
        b->lens[i] = b->lens[i + remove];
    }
    b->count -= remove;
}