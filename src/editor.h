#ifndef VIA_EDITOR_H
#define VIA_EDITOR_H
#include <stdbool.h>
#include "buffer.h"
#include "option.h"
#include "theme.h"
#ifndef CTRL
#define CTRL(x) ((x) & 0x1f)
#endif
#define VIA_CMD_MAX 511
#define VIA_MSG_MAX 255
#define VIA_TAB_MAX 64
typedef enum {
    MODE_NORMAL,
    MODE_INSERT,
    MODE_VISUAL,
    MODE_VISUAL_LINE,
    MODE_VISUAL_BLOCK,
    MODE_CMD,
    MODE_SEARCH,
    MODE_REPLACE,
} Mode;
typedef struct {
    int start_row, start_col;
    int end_row, end_col;
    bool active;
    bool linewise;
} Visual;
typedef struct {
    int type;
    int row, col;
    char *data;
    int len;
} UndoEntry;
typedef struct {
    UndoEntry *entries;
    int count;
    int cap;
    int pos;
} UndoStack;
typedef struct {
    char *text;
    int len;
    bool linewise;
} Register;
typedef struct Editor {
    Buffer *buf;
    char *filename;
    char *filetype;
    bool modified;
    int row, col;
    int top_row, left_col;
    int goal_col;
    Mode mode;
    bool running;
    char cmd_buf[VIA_CMD_MAX + 1];
    int cmd_len;
    int cmd_pos;
    bool cmd_active;
    char last_search[VIA_CMD_MAX + 1];
    int search_len;
    bool search_backward;
    char msg[VIA_MSG_MAX + 1];
    int msg_timer;
    Register regs[10];
    Register named_regs[26];
    UndoStack *undo;
    int undo_save_pos;
    int undolevels;
    int marks[26][2];
    bool mark_set[26];
    Visual visual;
    int count;
    int op_pending;
    int pending_find;
    char pending_find_char;
    bool pending_g;
    bool pending_z;
    bool pending_replace;
    bool pending_mark_set;
    bool pending_mark_jump;
    char last_text[1024];
    int last_text_len;
    int last_op;
    int last_motion;
    int last_count;
    char last_find_char;
    bool last_find_fwd;
    bool last_find_till;
    int tabstop;
    bool show_number;
    bool show_list;
    int scrolloff;
    int shiftwidth;
    bool mouse_dragging;
    bool mouse_drag_moved;
    int mouse_drag_row, mouse_drag_col;
    Options opts;
    Theme theme;
} Editor;
Editor *ed_new(void);
void ed_free(Editor *e);
void ed_run(Editor *e);
void ed_open(Editor *e, const char *path);
void ed_msg(Editor *e, const char *fmt, ...);
void ed_msg_clear(Editor *e);
void ed_scroll(Editor *e);
void ed_fix_cursor(Editor *e);
void ed_set_mode(Editor *e, Mode m);
void ed_quit(Editor *e);
Buffer *ed_buf(Editor *e);
int ed_row(Editor *e);
int ed_col(Editor *e);
int ed_top(Editor *e);
int ed_left(Editor *e);
int ed_rows(void);
int ed_cols(void);
#endif
