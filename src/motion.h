#ifndef VIA_MOTION_H
#define VIA_MOTION_H
#include "editor.h"
void motion_left(Editor *e, int n);
void motion_right(Editor *e, int n);
void motion_up(Editor *e, int n);
void motion_down(Editor *e, int n);
void motion_bol(Editor *e);
void motion_eol(Editor *e);
void motion_first_non_blank(Editor *e);
void motion_word_next(Editor *e, int n);
void motion_word_prev(Editor *e, int n);
void motion_WORD_next(Editor *e, int n);
void motion_WORD_prev(Editor *e, int n);
void motion_end_word(Editor *e, int n);
void motion_end_WORD(Editor *e, int n);
void motion_goto_line(Editor *e, int n);
void motion_goto_first(Editor *e);
void motion_goto_last(Editor *e);
void motion_find_next(Editor *e, char c);
void motion_find_prev(Editor *e, char c);
void motion_till_next(Editor *e, char c);
void motion_till_prev(Editor *e, char c);
void motion_repeat_find(Editor *e);
void motion_repeat_find_rev(Editor *e);
void motion_match(Editor *e);
void motion_page_down(Editor *e);
void motion_page_up(Editor *e);
void motion_half_down(Editor *e);
void motion_half_up(Editor *e);
void motion_scroll_down(Editor *e);
void motion_scroll_up(Editor *e);
void motion_screen_top(Editor *e);
void motion_screen_mid(Editor *e);
void motion_screen_bot(Editor *e);
void motion_goto_mark(Editor *e, char c);
void motion_goto_mark_line(Editor *e, char c);
int motion_is_linewise(int key);
void do_motion(Editor *e, int key, int count);
#endif
