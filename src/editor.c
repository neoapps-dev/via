#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <curses.h>
#include "editor.h"
#include "motion.h"
#include "change.h"
#include "render.h"
#include "input.h"
#include "ex.h"
Editor *ed_new(void)
{
    Editor *e = calloc(1, sizeof(Editor));
    e->buf = buf_new();
    e->running = true;
    e->mode = MODE_NORMAL;
    e->tabstop = 8;
    e->shiftwidth = 8;
    e->scrolloff = 0;
    e->show_number = false;
    e->show_list = false;
    e->goal_col = 0;
    e->undo = calloc(1, sizeof(UndoStack));
    e->undo_save_pos = 0;
    e->undolevels = 1000;
    e->last_find_char = 0;
    opt_init();
    via_theme = via_theme;
    theme_default();
    return e;
}

void ed_free(Editor *e)
{
    if (e->filename) free(e->filename);
    for (int i = 0; i < 10; i++)
        if (e->regs[i].text) free(e->regs[i].text);
    for (int i = 0; i < 26; i++)
        if (e->named_regs[i].text) free(e->named_regs[i].text);
    if (e->undo) {
        for (int i = 0; i < e->undo->count; i++)
            if (e->undo->entries[i].data) free(e->undo->entries[i].data);
        free(e->undo->entries);
        free(e->undo);
    }
    buf_free(e->buf);
    free(e);
}

void ed_msg(Editor *e, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(e->msg, VIA_MSG_MAX, fmt, ap);
    va_end(ap);
    e->msg_timer = 500;
}

void ed_msg_clear(Editor *e)
{
    e->msg[0] = '\0';
    e->msg_timer = 0;
}

void ed_scroll(Editor *e)
{
    int text_rows = LINES - 1;
    if (e->row < e->top_row + e->scrolloff)
        e->top_row = e->row - e->scrolloff;
    if (e->row > e->top_row + text_rows - 1 - e->scrolloff)
        e->top_row = e->row - text_rows + 1 + e->scrolloff;
    if (e->top_row < 0) e->top_row = 0;
    if (e->top_row > buf_count(e->buf) - text_rows)
        e->top_row = buf_count(e->buf) - text_rows;
    if (e->top_row < 0) e->top_row = 0;
}

void ed_fix_cursor(Editor *e)
{
    if (e->row < 0) e->row = 0;
    if (e->row >= buf_count(e->buf)) e->row = buf_count(e->buf) - 1;
    int max = buf_len(e->buf, e->row);
    if (e->col > max) e->col = max;
    if (e->col < 0) e->col = 0;
}

void ed_set_mode(Editor *e, Mode m)
{
    e->mode = m;
    e->pending_g = false;
    e->pending_z = false;
    e->pending_find = 0;
    e->op_pending = 0;
}

void ed_open(Editor *e, const char *path)
{
    if (e->filename) free(e->filename);
    e->filename = strdup(path);
    buf_free(e->buf);
    e->buf = buf_new();
    if (buf_load(e->buf, path) == 0) {
        e->modified = false;
        e->undo_save_pos = 0;
    } else {
        ed_msg(e, "New file");
    }
    e->row = 0;
    e->col = 0;
    e->top_row = 0;
    e->left_col = 0;
    e->goal_col = 0;
    if (e->undo) {
        for (int i = 0; i < e->undo->count; i++)
            if (e->undo->entries[i].data) free(e->undo->entries[i].data);
        free(e->undo->entries);
        e->undo->entries = NULL;
        e->undo->count = 0;
        e->undo->cap = 0;
        e->undo->pos = 0;
    }
}

void ed_quit(Editor *e)
{
    e->running = false;
}

static void handle_normal_key(Editor *e, int key)
{
    if (key == 27) {
        e->op_pending = 0;
        e->count = 0;
        e->pending_g = false;
        e->pending_z = false;
        e->pending_find = 0;
        return;
    }
    if (key == ':') {
        e->cmd_active = true;
        e->cmd_buf[0] = ':';
        e->cmd_len = 1;
        e->cmd_pos = 1;
        return;
    }
    if (key == '/') {
        e->cmd_active = true;
        e->cmd_buf[0] = '/';
        e->cmd_len = 1;
        e->cmd_pos = 1;
        e->search_backward = false;
        return;
    }
    if (key == '?') {
        e->cmd_active = true;
        e->cmd_buf[0] = '?';
        e->cmd_len = 1;
        e->cmd_pos = 1;
        e->search_backward = true;
        return;
    }
    if (e->pending_mark_set) {
        e->pending_mark_set = false;
        if (key >= 'a' && key <= 'z') {
            int idx = key - 'a';
            e->marks[idx][0] = e->row;
            e->marks[idx][1] = e->col;
            e->mark_set[idx] = true;
        }
        return;
    }
    if (e->pending_mark_jump) {
        e->pending_mark_jump = false;
        if (key >= 'a' && key <= 'z')
            motion_goto_mark_line(e, (char)key);
        return;
    }
    if (e->pending_replace) {
        e->pending_replace = false;
        change_replace_char(e, (char)key);
        return;
    }
    if (e->pending_z) {
        e->pending_z = false;
        if (key == 't') motion_screen_top(e);
        else if (key == 'z') motion_screen_mid(e);
        else if (key == 'b') motion_screen_bot(e);
        return;
    }
    if (e->pending_g) {
        e->pending_g = false;
        if (key == 'g') motion_goto_first(e);
        return;
    }
    if (e->pending_find) {
        e->pending_find_char = (char)key;
        int pf = e->pending_find;
        e->pending_find = 0;
        if (e->op_pending) {
            int op = e->op_pending;
            int count = e->count > 0 ? e->count : 1;
            e->op_pending = 0;
            e->count = 0;
            int sr = e->row, sc = e->col;
            if (pf == 'f') motion_find_next(e, (char)key);
            else if (pf == 'F') motion_find_prev(e, (char)key);
            else if (pf == 't') motion_till_next(e, (char)key);
            else if (pf == 'T') motion_till_prev(e, (char)key);
            if (op == 'd') op_delete(e, count, pf);
            else if (op == 'c') op_change(e, count, pf);
            else if (op == 'y') op_yank(e, count, pf);
            else { e->row = sr; e->col = sc; }
            return;
        }
        if (pf == 'f') motion_find_next(e, (char)key);
        else if (pf == 'F') motion_find_prev(e, (char)key);
        else if (pf == 't') motion_till_next(e, (char)key);
        else if (pf == 'T') motion_till_prev(e, (char)key);
        return;
    }
    if (key == 'r') {
        e->pending_replace = true;
        return;
    }
    if (key >= '1' && key <= '9') {
        e->count = e->count * 10 + (key - '0');
        return;
    }
    if (key == '0' && e->count > 0) {
        e->count = e->count * 10;
        return;
    }
    if (e->op_pending) {
        if (key == e->op_pending) {
            e->count = 0;
            e->op_pending = 0;
            if (key == 'd') change_delete_line(e);
            else if (key == 'c') change_change_line(e);
            else if (key == 'y') change_yank_line(e);
            else if (key == '>') change_shift_right(e);
            else if (key == '<') change_shift_left(e);
            return;
        }
        if (key == 'f' || key == 'F' || key == 't' || key == 'T') {
            e->pending_find = key;
            return;
        }
        if (key == 'i' || key == 'a' || key == 'I' || key == 'A' ||
            key == 'o' || key == 'O') {
            e->op_pending = 0;
            e->count = 0;
            handle_normal_key(e, key);
            return;
        }
        int op = e->op_pending;
        int count = e->count > 0 ? e->count : 1;
        e->op_pending = 0;
        e->count = 0;
        if (op == 'd') op_delete(e, count, key);
        else if (op == 'c') op_change(e, count, key);
        else if (op == 'y') op_yank(e, count, key);
        else if (op == '>') op_shift_right(e, count, key);
        else if (op == '<') op_shift_left(e, count, key);
        return;
    }
    int count = e->count > 0 ? e->count : 1;
    e->count = 0;
    switch (key) {
    case 'h': case KEY_LEFT: motion_left(e, count); break;
    case 'j': case KEY_DOWN: motion_down(e, count); break;
    case 'k': case KEY_UP: motion_up(e, count); break;
    case 'l': case KEY_RIGHT: motion_right(e, count); break;
    case '0': motion_bol(e); break;
    case '$': case KEY_END: motion_eol(e); break;
    case '^': motion_first_non_blank(e); break;
    case 'w': motion_word_next(e, count); break;
    case 'W': motion_WORD_next(e, count); break;
    case 'b': motion_word_prev(e, count); break;
    case 'B': motion_WORD_prev(e, count); break;
    case 'e': motion_end_word(e, count); break;
    case 'E': motion_end_WORD(e, count); break;
    case 'G': motion_goto_last(e); break;
    case '%': motion_match(e); break;
    case 'f': e->pending_find = 'f'; break;
    case 'F': e->pending_find = 'F'; break;
    case 't': e->pending_find = 't'; break;
    case 'T': e->pending_find = 'T'; break;
    case ';': motion_repeat_find(e); break;
    case ',': motion_repeat_find_rev(e); break;
    case 'n': if (e->last_search[0]) ex_cmd_number(e, e->last_search); break;
    case 'N': e->search_backward = !e->search_backward;
              if (e->last_search[0]) ex_cmd_number(e, e->last_search);
              e->search_backward = !e->search_backward; break;
    case KEY_NPAGE: motion_page_down(e); break;
    case KEY_PPAGE: motion_page_up(e); break;
    case KEY_HOME: motion_bol(e); break;
    case 'd': e->op_pending = 'd'; break;
    case 'y': e->op_pending = 'y'; break;
    case 'c': e->op_pending = 'c'; break;
    case '>': e->op_pending = '>'; break;
    case '<': e->op_pending = '<'; break;
    case 'i': change_insert(e); break;
    case 'a': change_append(e); break;
    case 'I': change_insert_bol(e); break;
    case 'A': change_append_eol(e); break;
    case 'o': change_open_below(e); break;
    case 'O': change_open_above(e); break;
    case 'x': change_delete_char(e); break;
    case 'X': change_delete_char_back(e); break;
    case 's': change_substitute_char(e); break;
    case 'S': change_substitute_line(e); break;
    case 'D': change_delete_to_eol(e); break;
    case 'C': change_delete_to_eol(e); change_insert(e); break;
    case 'p': change_paste_after(e, count); break;
    case 'P': change_paste_before(e, count); break;
    case 'J': change_join(e); break;
    case 'u': change_undo(e); break;
    case CTRL('R'): change_redo(e); break;
    case '.': change_repeat(e); break;
    case '~': change_toggle_case(e); break;
    case 'v': ed_set_mode(e, MODE_VISUAL);
              e->visual.active = true;
              e->visual.start_row = e->row;
              e->visual.start_col = e->col;
              e->visual.linewise = false; break;
    case 'V': ed_set_mode(e, MODE_VISUAL_LINE);
              e->visual.active = true;
              e->visual.start_row = e->row;
              e->visual.start_col = 0;
              e->visual.linewise = true; break;
    case 'z': e->pending_z = true; break;
    case 'g': e->pending_g = true; break;
    case 'm': e->pending_mark_set = true; break;
    case '\'': e->pending_mark_jump = true; break;
    case 'H': motion_screen_top(e); break;
    case 'M': motion_screen_mid(e); break;
    case 'L': motion_screen_bot(e); break;
    case CTRL('F'): motion_page_down(e); break;
    case CTRL('B'): motion_page_up(e); break;
    case CTRL('D'): motion_half_down(e); break;
    case CTRL('U'): motion_half_up(e); break;
    case CTRL('E'): motion_scroll_down(e); break;
    case CTRL('Y'): motion_scroll_up(e); break;
    case CTRL('L'): clearok(stdscr, TRUE); break;
    case CTRL('K'): break;
    }
}

static void handle_insert_key(Editor *e, int key)
{
    if (key == 27) {
        ed_set_mode(e, MODE_NORMAL);
        return;
    }
    if (key == KEY_BACKSPACE || key == 127 || key == 8) {
        change_delete_char_back(e);
        return;
    }
    if (key == '\n' || key == KEY_ENTER) {
        vi_insert_char(e, '\n');
        return;
    }
    if (key == KEY_LEFT) { motion_left(e, 1); return; }
    if (key == KEY_RIGHT) { motion_right(e, 1); return; }
    if (key == KEY_UP) { motion_up(e, 1); return; }
    if (key == KEY_DOWN) { motion_down(e, 1); return; }
    if (key == KEY_HOME) { motion_bol(e); return; }
    if (key == KEY_END) { motion_eol(e); return; }
    if (key == KEY_NPAGE) { motion_page_down(e); return; }
    if (key == KEY_PPAGE) { motion_page_up(e); return; }
    if (key == '\t' || key == 9) {
        vi_insert_char(e, '\t');
        return;
    }
    if (key >= 32 && key <= 126) {
        vi_insert_char(e, (char)key);
        return;
    }
}

static void handle_replace_key(Editor *e, int key)
{
    if (key == 27) {
        ed_set_mode(e, MODE_NORMAL);
        return;
    }
    if (key >= 32 && key <= 126) {
        change_replace_char(e, (char)key);
        ed_set_mode(e, MODE_NORMAL);
        return;
    }
}

static void handle_visual_key(Editor *e, int key)
{
    if (key == 27 || key == 'v' || key == 'V') {
        e->visual.active = false;
        ed_set_mode(e, MODE_NORMAL);
        return;
    }
    e->visual.end_row = e->row;
    e->visual.end_col = e->col;
    if (key == 'd' || key == 'x') {
        int r1 = e->visual.start_row, c1 = e->visual.start_col;
        int r2 = e->row, c2 = e->col;
        if (r1 > r2 || (r1 == r2 && c1 > c2)) {
            int t = r1; r1 = r2; r2 = t;
            t = c1; c1 = c2; c2 = t;
        }
        if (e->visual.linewise) {
            for (int r = r2; r >= r1; r--) {
                char *l = strdup(buf_line(e->buf, r));
                int llen = buf_len(e->buf, r);
                buf_delete_line(e->buf, r);
                undo_push(e, 7, r, 0, l, llen);
                free(l);
            }
            e->row = r1 < buf_count(e->buf) ? r1 : buf_count(e->buf) - 1;
            e->col = 0;
        } else {
            if (r1 == r2) {
                int len = c2 - c1 + 1;
                char *t = malloc((size_t)(len + 1));
                memcpy(t, buf_line(e->buf, r1) + c1, (size_t)len);
                t[len] = '\0';
                buf_delete_range(e->buf, r1, c1, r2, c2 + 1);
                undo_push(e, 8, r1, c1, t, len);
                free(t);
            } else {
                int del_len = (buf_len(e->buf, r1) - c1) + (c2 + 1);
                for (int r = r1 + 1; r < r2; r++)
                    del_len += buf_len(e->buf, r);
                char *t = malloc((size_t)(del_len + 1));
                int p = 0;
                memcpy(t + p, buf_line(e->buf, r1) + c1, (size_t)(buf_len(e->buf, r1) - c1));
                p += buf_len(e->buf, r1) - c1;
                for (int r = r1 + 1; r < r2; r++) {
                    memcpy(t + p, buf_line(e->buf, r), (size_t)buf_len(e->buf, r));
                    p += buf_len(e->buf, r);
                }
                memcpy(t + p, buf_line(e->buf, r2), (size_t)(c2 + 1));
                p += c2 + 1;
                t[del_len] = '\0';
                buf_delete_range(e->buf, r1, c1, r2, c2 + 1);
                undo_push(e, 8, r1, c1, t, del_len);
                free(t);
            }
            e->row = r1;
            e->col = c1;
        }
        e->goal_col = e->col;
        e->visual.active = false;
        ed_set_mode(e, MODE_NORMAL);
        return;
    }
    if (key == 'y') {
        int r1 = e->visual.start_row, c1 = e->visual.start_col;
        int r2 = e->row, c2 = e->col;
        if (r1 > r2 || (r1 == r2 && c1 > c2)) {
            int t = r1; r1 = r2; r2 = t;
            t = c1; c1 = c2; c2 = t;
        }
        if (e->regs[0].text) free(e->regs[0].text);
        int total = 0;
        if (e->visual.linewise) {
            for (int r = r1; r <= r2; r++) total += buf_len(e->buf, r) + 1;
            total--;
        } else if (r1 == r2) {
            total = c2 - c1 + 1;
        } else {
            total = buf_len(e->buf, r1) - c1 + c2 + 1;
            for (int r = r1 + 1; r < r2; r++) total += buf_len(e->buf, r);
        }
        char *y = malloc((size_t)(total + 1));
        int pos = 0;
        if (e->visual.linewise) {
            for (int r = r1; r <= r2; r++) {
                memcpy(y + pos, buf_line(e->buf, r), (size_t)buf_len(e->buf, r));
                pos += buf_len(e->buf, r);
                if (r < r2) { y[pos] = '\n'; pos++; }
            }
            e->regs[0].linewise = true;
        } else if (r1 == r2) {
            memcpy(y, buf_line(e->buf, r1) + c1, (size_t)total);
            e->regs[0].linewise = false;
        } else {
            memcpy(y + pos, buf_line(e->buf, r1) + c1, (size_t)(buf_len(e->buf, r1) - c1));
            pos += buf_len(e->buf, r1) - c1;
            for (int r = r1 + 1; r < r2; r++) {
                memcpy(y + pos, buf_line(e->buf, r), (size_t)buf_len(e->buf, r));
                pos += buf_len(e->buf, r);
            }
            memcpy(y + pos, buf_line(e->buf, r2), (size_t)(c2 + 1));
            pos += c2 + 1;
            e->regs[0].linewise = false;
        }
        y[total] = '\0';
        e->regs[0].text = y;
        e->regs[0].len = total;
        e->visual.active = false;
        ed_set_mode(e, MODE_NORMAL);
        return;
    }
    if (key == 'c') {
        handle_visual_key(e, 'd');
        if (e->mode == MODE_NORMAL) ed_set_mode(e, MODE_INSERT);
        return;
    }
    if (key == '>') {
        int r1 = e->visual.start_row;
        int r2 = e->row;
        if (r1 > r2) { int t = r1; r1 = r2; r2 = t; }
        for (int r = r1; r <= r2; r++)
            for (int i = 0; i < e->shiftwidth; i++) {
                buf_insert_char(e->buf, r, 0, ' ');
                undo_push(e, 1, r, i, " ", 1);
            }
        e->visual.active = false;
        ed_set_mode(e, MODE_NORMAL);
        return;
    }
    if (key == '<') {
        int r1 = e->visual.start_row;
        int r2 = e->row;
        if (r1 > r2) { int t = r1; r1 = r2; r2 = t; }
        for (int r = r1; r <= r2; r++) {
            char *line = buf_line(e->buf, r);
            int sw = e->shiftwidth < buf_len(e->buf, r) ? e->shiftwidth : buf_len(e->buf, r);
            int spaces = 0;
            while (spaces < sw && line[spaces] == ' ') spaces++;
            for (int i = 0; i < spaces; i++) {
                buf_delete_char(e->buf, r, 0);
                undo_push(e, 4, r, 0, " ", 1);
            }
        }
        e->visual.active = false;
        ed_set_mode(e, MODE_NORMAL);
        return;
    }
    handle_normal_key(e, key);
    if (e->mode != MODE_VISUAL && e->mode != MODE_VISUAL_LINE && e->mode != MODE_VISUAL_BLOCK) {
        e->visual.active = false;
    }
}

void ed_run(Editor *e)
{
    render_init();
    render_draw(e);
    while (e->running) {
        if (e->msg_timer > 0) {
            e->msg_timer--;
            if (e->msg_timer == 0) ed_msg_clear(e);
        }
        render_draw(e);
        int key = input_read(e);
        if (key == ERR) continue;
        if (key == KEY_MOUSE) {
            MEVENT me;
            if (getmouse(&me) == OK) {
                int text_rows = LINES - 1;
                if (me.bstate & BUTTON1_PRESSED) {
                    e->mouse_dragging = true;
                    e->mouse_drag_moved = false;
                    int num_width = e->show_number ? 7 : 0;
                    e->mouse_drag_row = e->top_row + me.y;
                    e->mouse_drag_col = me.x - num_width;
                    if (e->mouse_drag_col < 0) e->mouse_drag_col = 0;
                } else if (me.bstate & REPORT_MOUSE_POSITION) {
                    if (e->mouse_dragging && me.y < text_rows) {
                        if (!e->mouse_drag_moved) {
                            e->mouse_drag_moved = true;
                            ed_set_mode(e, MODE_VISUAL);
                            e->visual.active = true;
                            e->visual.start_row = e->mouse_drag_row;
                            e->visual.start_col = e->mouse_drag_col;
                            e->visual.linewise = false;
                            e->row = e->mouse_drag_row;
                            e->col = e->mouse_drag_col;
                            e->goal_col = e->mouse_drag_col;
                        }
                        int num_width = e->show_number ? 7 : 0;
                        int br = e->top_row + me.y;
                        int bc = me.x - num_width;
                        if (bc < 0) bc = 0;
                        if (br >= 0 && br < buf_count(e->buf)) {
                            int max = buf_len(e->buf, br);
                            if (bc > max) bc = max;
                            e->row = br;
                            e->col = bc;
                            e->goal_col = bc;
                        }
                    }
                } else if (me.bstate & BUTTON1_RELEASED) {
                    e->mouse_dragging = false;
                    if (!e->mouse_drag_moved) {
                        e->visual.active = false;
                        ed_set_mode(e, MODE_NORMAL);
                        e->row = e->mouse_drag_row;
                        e->col = e->mouse_drag_col;
                        if (e->col < 0) e->col = 0;
                        int max = buf_len(e->buf, e->row);
                        if (e->col > max) e->col = max;
                        e->goal_col = e->col;
                    }
                } else if (me.bstate & BUTTON4_PRESSED) {
                    if (e->top_row > 0) {
                        e->top_row--;
                        if (e->row > 0) e->row--;
                        e->goal_col = e->col;
                    }
                } else if (me.bstate & BUTTON5_PRESSED) {
                    int last_top = buf_count(e->buf) - text_rows;
                    if (e->top_row < last_top) {
                        e->top_row++;
                        if (e->row < buf_count(e->buf) - 1) e->row++;
                        e->goal_col = e->col;
                    }
                }
            }
            ed_fix_cursor(e);
            continue;
        }
        if (e->cmd_active) {
            ex_handle_key(e, key);
            continue;
        }
        switch (e->mode) {
        case MODE_NORMAL:
            handle_normal_key(e, key);
            ed_fix_cursor(e);
            break;
        case MODE_INSERT:
            handle_insert_key(e, key);
            ed_fix_cursor(e);
            break;
        case MODE_REPLACE:
            handle_replace_key(e, key);
            ed_fix_cursor(e);
            break;
        case MODE_VISUAL:
        case MODE_VISUAL_LINE:
            handle_visual_key(e, key);
            ed_fix_cursor(e);
            break;
        default:
            break;
        }
        if (key == KEY_RESIZE) {
            render_resize();
            clearok(stdscr, TRUE);
        }
    }
    render_end();
}
