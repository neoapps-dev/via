#ifndef VIA_BUFFER_H
#define VIA_BUFFER_H
typedef struct {
    char **lines;
    int *lens;
    int count;
    int cap;
} Buffer;
Buffer *buf_new(void);
void buf_free(Buffer *b);
int buf_load(Buffer *b, const char *path);
int buf_save(Buffer *b, const char *path);
void buf_insert_char(Buffer *b, int row, int col, char c);
void buf_delete_char(Buffer *b, int row, int col);
void buf_insert_line(Buffer *b, int row);
void buf_delete_line(Buffer *b, int row);
void buf_join(Buffer *b, int row);
void buf_split(Buffer *b, int row, int col);
void buf_clear(Buffer *b);
char *buf_line(Buffer *b, int row);
int buf_len(Buffer *b, int row);
int buf_count(Buffer *b);
void buf_insert_str(Buffer *b, int row, int col, const char *s, int len);
void buf_delete_range(Buffer *b, int r1, int c1, int r2, int c2);
#endif
