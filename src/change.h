#ifndef VIA_CHANGE_H
#define VIA_CHANGE_H
#include "editor.h"
void change_insert(Editor *e);
void change_append(Editor *e);
void change_insert_bol(Editor *e);
void change_append_eol(Editor *e);
void change_open_above(Editor *e);
void change_open_below(Editor *e);
void change_replace_char(Editor *e, char c);
void change_enter_replace(Editor *e);
void change_delete_char(Editor *e);
void change_delete_char_back(Editor *e);
void change_delete_line(Editor *e);
void change_delete_to_eol(Editor *e);
void change_yank_line(Editor *e);
void change_paste_after(Editor *e, int n);
void change_paste_before(Editor *e, int n);
void change_change_line(Editor *e);
void change_join(Editor *e);
void change_undo(Editor *e);
void change_redo(Editor *e);
void change_shift_right(Editor *e);
void change_shift_left(Editor *e);
void change_toggle_case(Editor *e);
void change_substitute_char(Editor *e);
void change_substitute_line(Editor *e);
void change_repeat(Editor *e);
void op_delete(Editor *e, int count, int motion);
void op_change(Editor *e, int count, int motion);
void op_yank(Editor *e, int count, int motion);
void op_shift_right(Editor *e, int count, int motion);
void op_shift_left(Editor *e, int count, int motion);
void undo_push(Editor *e, int type, int row, int col, const char *data, int len);
void vi_insert_char(Editor *e, char c);
#endif
