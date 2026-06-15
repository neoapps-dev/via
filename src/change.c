#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "change.h"
#include "motion.h"
void undo_push(Editor *e, int type, int row, int col, const char *data, int len)
{
    UndoStack *u = e->undo;
    u->count = u->pos;
    if (u->count >= u->cap) {
        u->cap = u->cap ? u->cap * 2 : 64;
        u->entries = realloc(u->entries, (size_t)u->cap * sizeof(UndoEntry));
    }
    UndoEntry *r = &u->entries[u->count++];
    r->type = type;
    r->row = row;
    r->col = col;
    r->data = NULL;
    r->len = len;
    if (len > 0 && data) {
        r->data = malloc((size_t)(len + 1));
        memcpy(r->data, data, (size_t)len);
        r->data[len] = '\0';
    }
    u->pos = u->count;
    e->modified = (u->pos != e->undo_save_pos);
}

void vi_insert_char(Editor *e, char c)
{
    if (c == '\n') {
        buf_split(e->buf, e->row, e->col);
        undo_push(e, 0, e->row, e->col, "\n", 1);
        e->row++;
        e->col = 0;
    } else if (c == '\t') {
        int n = e->tabstop - (e->col % e->tabstop);
        for (int i = 0; i < n; i++) {
            buf_insert_char(e->buf, e->row, e->col, ' ');
            undo_push(e, 1, e->row, e->col, " ", 1);
            e->col++;
        }
    } else {
        buf_insert_char(e->buf, e->row, e->col, c);
        undo_push(e, 1, e->row, e->col, &c, 1);
        e->col++;
    }
}

void change_insert(Editor *e)
{
    ed_set_mode(e, MODE_INSERT);
}

void change_append(Editor *e)
{
    int len = buf_len(e->buf, e->row);
    if (e->col < len) e->col++;
    ed_set_mode(e, MODE_INSERT);
}

void change_insert_bol(Editor *e)
{
    motion_first_non_blank(e);
    ed_set_mode(e, MODE_INSERT);
}

void change_append_eol(Editor *e)
{
    motion_eol(e);
    ed_set_mode(e, MODE_INSERT);
}

void change_open_above(Editor *e)
{
    buf_insert_line(e->buf, e->row);
    undo_push(e, 2, e->row, 0, NULL, 0);
    ed_set_mode(e, MODE_INSERT);
}

void change_open_below(Editor *e)
{
    buf_insert_line(e->buf, e->row + 1);
    undo_push(e, 2, e->row + 1, 0, NULL, 0);
    e->row++;
    e->col = 0;
    e->goal_col = 0;
    ed_set_mode(e, MODE_INSERT);
}

void change_replace_char(Editor *e, char c)
{
    int len = buf_len(e->buf, e->row);
    if (e->col < len) {
        char old = buf_line(e->buf, e->row)[e->col];
        buf_delete_char(e->buf, e->row, e->col);
        buf_insert_char(e->buf, e->row, e->col, c);
        char data[2] = {old, c};
        undo_push(e, 3, e->row, e->col, data, 2);
    }
}

void change_enter_replace(Editor *e)
{
    ed_set_mode(e, MODE_REPLACE);
}

void change_delete_char(Editor *e)
{
    int len = buf_len(e->buf, e->row);
    if (e->col < len) {
        char c = buf_line(e->buf, e->row)[e->col];
        buf_delete_char(e->buf, e->row, e->col);
        undo_push(e, 4, e->row, e->col, &c, 1);
    } else if (e->row < buf_count(e->buf) - 1) {
        buf_delete_char(e->buf, e->row, e->col);
        undo_push(e, 5, e->row, e->col, NULL, 0);
    }
}

void change_delete_char_back(Editor *e)
{
    if (e->col > 0) {
        e->col--;
        char c = buf_line(e->buf, e->row)[e->col];
        buf_delete_char(e->buf, e->row, e->col);
        undo_push(e, 4, e->row, e->col, &c, 1);
    } else if (e->row > 0) {
        int old_col = buf_len(e->buf, e->row - 1);
        buf_join(e->buf, e->row - 1);
        undo_push(e, 6, e->row - 1, old_col, NULL, 0);
        e->row--;
        e->col = old_col;
    }
    e->goal_col = e->col;
}

void change_delete_line(Editor *e)
{
    int c = buf_count(e->buf);
    if (c <= 1) {
        buf_clear(e->buf);
        undo_push(e, 7, 0, 0, NULL, 0);
        e->row = 0;
        e->col = 0;
        return;
    }
    char *dl = strdup(buf_line(e->buf, e->row));
    int dl_len = buf_len(e->buf, e->row);
    int row = e->row;
    buf_delete_line(e->buf, e->row);
    undo_push(e, 7, row, 0, dl, dl_len);
    free(dl);
    if (e->row >= buf_count(e->buf)) e->row = buf_count(e->buf) - 1;
    int max = buf_len(e->buf, e->row);
    if (e->col > max) e->col = max;
    e->goal_col = e->col;
}

void change_delete_to_eol(Editor *e)
{
    int len = buf_len(e->buf, e->row);
    if (e->col < len) {
        int dl = len - e->col;
        char *deleted = malloc((size_t)(dl + 1));
        memcpy(deleted, buf_line(e->buf, e->row) + e->col, (size_t)dl);
        deleted[dl] = '\0';
        buf_line(e->buf, e->row)[e->col] = '\0';
        e->buf->lens[e->row] = e->col;
        undo_push(e, 8, e->row, e->col, deleted, dl);
        free(deleted);
    }
}

void change_yank_line(Editor *e)
{
    int r = buf_count(e->buf);
    int total = 0;
    for (int i = 0; i < r; i++) total += buf_len(e->buf, i) + 1;
    if (total > 0) total--;
    if (e->regs[0].text) free(e->regs[0].text);
    char *y = malloc((size_t)(total + 1));
    int pos = 0;
    for (int i = 0; i < r; i++) {
        memcpy(y + pos, buf_line(e->buf, i), (size_t)buf_len(e->buf, i));
        pos += buf_len(e->buf, i);
        if (i < r - 1) { y[pos] = '\n'; pos++; }
    }
    y[pos] = '\0';
    e->regs[0].text = y;
    e->regs[0].len = total;
    e->regs[0].linewise = true;
}

void change_paste_after(Editor *e, int n)
{
    Register *r = &e->regs[0];
    if (!r->text || r->len == 0) return;
    if (r->linewise) {
        for (int i = 0; i < n; i++) {
            buf_insert_line(e->buf, e->row + 1);
            char *line = buf_line(e->buf, e->row + 1);
            int len = 0;
            while (len < r->len && r->text[len] != '\n') len++;
            free(line);
            e->buf->lines[e->row + 1] = strndup(r->text, (size_t)len);
            e->buf->lens[e->row + 1] = len;
            undo_push(e, 9, e->row + 1, 0, NULL, 0);
        }
    } else {
        for (int i = 0; i < n; i++) {
            buf_insert_str(e->buf, e->row, e->col + 1, r->text, r->len);
            undo_push(e, 10, e->row, e->col + 1, r->text, r->len);
        }
        e->col += r->len;
    }
}

void change_paste_before(Editor *e, int n)
{
    Register *r = &e->regs[0];
    if (!r->text || r->len == 0) return;
    if (r->linewise) {
        for (int i = 0; i < n; i++) {
            buf_insert_line(e->buf, e->row);
            char *line = buf_line(e->buf, e->row);
            int len = 0;
            while (len < r->len && r->text[len] != '\n') len++;
            free(line);
            e->buf->lines[e->row] = strndup(r->text, (size_t)len);
            e->buf->lens[e->row] = len;
            undo_push(e, 9, e->row, 0, NULL, 0);
            e->row++;
        }
    } else {
        for (int i = 0; i < n; i++) {
            buf_insert_str(e->buf, e->row, e->col, r->text, r->len);
            undo_push(e, 10, e->row, e->col, r->text, r->len);
        }
        e->col += r->len;
    }
}

void change_change_line(Editor *e)
{
    change_delete_line(e);
    ed_set_mode(e, MODE_INSERT);
}

void change_join(Editor *e)
{
    if (e->row >= buf_count(e->buf) - 1) return;
    int len = buf_len(e->buf, e->row);
    char *next = buf_line(e->buf, e->row + 1);
    int next_len = buf_len(e->buf, e->row + 1);
    if (next_len > 0) {
        buf_insert_char(e->buf, e->row, len, ' ');
        undo_push(e, 1, e->row, len, " ", 1);
    }
    for (int i = 0; i < next_len; i++) {
        buf_insert_char(e->buf, e->row, buf_len(e->buf, e->row), next[i]);
        undo_push(e, 1, e->row, buf_len(e->buf, e->row) - 1, &next[i], 1);
    }
    buf_delete_line(e->buf, e->row + 1);
    undo_push(e, 7, e->row + 1, 0, NULL, 0);
}

void change_undo(Editor *e)
{
    UndoStack *u = e->undo;
    if (u->pos <= 0) return;
    UndoEntry *r = &u->entries[--u->pos];
    switch (r->type) {
    case 0: { //neo: split (inserted \n)
        buf_join(e->buf, r->row);
        e->row = r->row;
        e->col = r->col;
        break;
    }
    case 1: //neo: inserted char
        if (r->len == 1 && r->data[0] == ' ') {
            int spaces = 0;
            while (u->pos > 0 && u->entries[u->pos - 1].type == 1 &&
                   u->entries[u->pos - 1].data[0] == ' ') {
                u->pos--;
                spaces++;
            }
            for (int i = 0; i <= spaces; i++) {
                if (e->col > 0) {
                    e->col--;
                    buf_delete_char(e->buf, r->row, r->col - (spaces - i));
                }
            }
        } else {
            for (int i = 0; i < r->len; i++) {
                if (e->col > 0) e->col--;
                buf_delete_char(e->buf, r->row, r->col);
            }
        }
        break;
    case 2: //neo: inserted line
        buf_delete_line(e->buf, r->row);
        if (e->row > 0) e->row--;
        break;
    case 3: { //neo: replaced char
        buf_delete_char(e->buf, r->row, r->col);
        buf_insert_char(e->buf, r->row, r->col, r->data[0]);
        break;
    }
    case 4: //neo: deleted char
        buf_insert_char(e->buf, r->row, r->col, r->data[0]);
        break;
    case 5: //neo: deleted at eol
        buf_join(e->buf, r->row);
        break;
    case 6: //neo: joined lines (backspace at bol)
        buf_split(e->buf, r->row, r->col);
        e->row = r->row + 1;
        e->col = 0;
        break;
    case 7: //neo: deleted line
        buf_insert_line(e->buf, r->row);
        free(e->buf->lines[r->row]);
        e->buf->lines[r->row] = strdup(r->data);
        e->buf->lens[r->row] = r->len;
        break;
    case 8: //neo: deleted to eol
        buf_insert_str(e->buf, r->row, r->col, r->data, r->len);
        break;
    case 9: //neo: pasted line (inserted line)
        buf_delete_line(e->buf, r->row);
        if (e->row > 0) e->row--;
        break;
    case 10: //neo: pasted text
        for (int i = 0; i < r->len; i++)
            buf_delete_char(e->buf, r->row, r->col);
        break;
    }
    e->modified = (u->pos != e->undo_save_pos);
}

void change_redo(Editor *e)
{
    UndoStack *u = e->undo;
    if (u->pos >= u->count) return;
    UndoEntry *r = &u->entries[u->pos];
    switch (r->type) {
    case 0:
        buf_split(e->buf, r->row, r->col);
        e->row = r->row + 1;
        e->col = 0;
        break;
    case 1:
        for (int i = 0; i < r->len; i++) {
            buf_insert_char(e->buf, r->row, r->col + i, r->data[i]);
        }
        e->row = r->row;
        e->col = r->col + r->len;
        break;
    case 2:
        buf_insert_line(e->buf, r->row);
        e->row = r->row;
        e->col = 0;
        break;
    case 3:
        buf_delete_char(e->buf, r->row, r->col);
        buf_insert_char(e->buf, r->row, r->col, r->data[1]);
        break;
    case 4:
        buf_delete_char(e->buf, r->row, r->col);
        break;
    case 5:
        buf_delete_char(e->buf, r->row, r->col);
        break;
    case 6:
        buf_join(e->buf, r->row);
        e->row = r->row;
        e->col = r->col;
        break;
    case 7:
        buf_delete_line(e->buf, r->row);
        if (e->row > 0) e->row--;
        break;
    case 8:
        buf_line(e->buf, r->row)[r->col] = '\0';
        e->buf->lens[r->row] = r->col;
        break;
    case 9:
        buf_insert_line(e->buf, r->row);
        free(e->buf->lines[r->row]);
        e->buf->lines[r->row] = strdup("");
        e->buf->lens[r->row] = 0;
        break;
    case 10:
        buf_insert_str(e->buf, r->row, r->col, r->data, r->len);
        break;
    }
    u->pos++;
    e->modified = (u->pos != e->undo_save_pos);
}

void change_shift_right(Editor *e)
{
    int sw = e->shiftwidth;
    for (int i = 0; i < sw; i++) {
        buf_insert_char(e->buf, e->row, 0, ' ');
        undo_push(e, 1, e->row, i, " ", 1);
    }
    e->col += sw;
    e->goal_col = e->col;
}

void change_shift_left(Editor *e)
{
    int sw = e->shiftwidth;
    char *line = buf_line(e->buf, e->row);
    int spaces = 0;
    while (spaces < sw && spaces < buf_len(e->buf, e->row) && line[spaces] == ' ')
        spaces++;
    for (int i = 0; i < spaces; i++) {
        buf_delete_char(e->buf, e->row, 0);
        undo_push(e, 4, e->row, 0, " ", 1);
    }
    e->col -= spaces;
    if (e->col < 0) e->col = 0;
    e->goal_col = e->col;
}

void change_toggle_case(Editor *e)
{
    char *line = buf_line(e->buf, e->row);
    int len = buf_len(e->buf, e->row);
    if (!line || e->col >= len) return;
    char c = line[e->col];
    if (islower((unsigned char)c)) {
        c = (char)toupper((unsigned char)c);
        buf_delete_char(e->buf, e->row, e->col);
        buf_insert_char(e->buf, e->row, e->col, c);
    } else if (isupper((unsigned char)c)) {
        c = (char)tolower((unsigned char)c);
        buf_delete_char(e->buf, e->row, e->col);
        buf_insert_char(e->buf, e->row, e->col, c);
    }
    if (e->col < len - 1) e->col++;
    e->goal_col = e->col;
}

void change_substitute_char(Editor *e)
{
    change_delete_char(e);
    ed_set_mode(e, MODE_INSERT);
}

void change_substitute_line(Editor *e)
{
    change_delete_line(e);
    ed_set_mode(e, MODE_INSERT);
}

static void remember_op(Editor *e, int op, int count, int motion)
{
    e->last_op = op;
    e->last_count = count;
    e->last_motion = motion;
}

static int min_i(int a, int b) { return a < b ? a : b; }
static int max_i(int a, int b) { return a > b ? a : b; }

void op_delete(Editor *e, int count, int motion)
{
    int sr = e->row, sc = e->col;
    do_motion(e, motion, count);
    int er = e->row, ec = e->col;
    if (sr == er && sc == ec) return;
    remember_op(e, 'd', count, motion);
    if (motion_is_linewise(motion)) {
        int r1 = min_i(sr, er);
        int r2 = max_i(sr, er);
        for (int r = r2; r >= r1; r--) {
            char *line = strdup(buf_line(e->buf, r));
            int llen = buf_len(e->buf, r);
            buf_delete_line(e->buf, r);
            undo_push(e, 7, r, 0, line, llen);
            free(line);
        }
        if (r1 >= buf_count(e->buf)) e->row = buf_count(e->buf) - 1;
        else e->row = r1;
        e->col = 0;
    } else {
        int r1, c1, r2, c2;
        if (sr < er || (sr == er && sc <= ec)) {
            r1 = sr; c1 = sc; r2 = er; c2 = ec;
        } else {
            r1 = er; c1 = ec; r2 = sr; c2 = sc;
        }
        if (motion == 'e' || motion == 'E') c2++;
        if (r1 == r2) {
            int del_len = c2 - c1;
            char *t = malloc((size_t)(del_len + 1));
            memcpy(t, buf_line(e->buf, r1) + c1, (size_t)del_len);
            t[del_len] = '\0';
            buf_delete_range(e->buf, r1, c1, r2, c2);
            undo_push(e, 8, r1, c1, t, del_len);
            free(t);
            e->row = r1; e->col = c1;
        } else {
            char *t = NULL;
            int del_len = 0;
            for (int r = r1; r <= r2; r++)
                del_len += (r == r1 ? buf_len(e->buf, r) - c1 :
                           (r == r2 ? c2 : buf_len(e->buf, r)));
            t = malloc((size_t)(del_len + 1));
            int pos = 0;
            for (int r = r1; r <= r2; r++) {
                int start = (r == r1) ? c1 : 0;
                int end = (r == r2) ? c2 : buf_len(e->buf, r);
                int n = end - start;
                memcpy(t + pos, buf_line(e->buf, r) + start, (size_t)n);
                pos += n;
            }
            t[del_len] = '\0';
            buf_delete_range(e->buf, r1, c1, r2, c2);
            undo_push(e, 8, r1, c1, t, del_len);
            free(t);
            e->row = r1; e->col = c1;
        }
    }
    e->goal_col = e->col;
}

void op_change(Editor *e, int count, int motion)
{
    op_delete(e, count, motion);
    ed_set_mode(e, MODE_INSERT);
}

void op_yank(Editor *e, int count, int motion)
{
    int sr = e->row, sc = e->col;
    do_motion(e, motion, count);
    int er = e->row, ec = e->col;
    if (sr == er && sc == ec) return;
    remember_op(e, 'y', count, motion);
    if (e->regs[0].text) free(e->regs[0].text);
    if (motion_is_linewise(motion)) {
        int r1 = min_i(sr, er);
        int r2 = max_i(sr, er);
        int total = 0;
        for (int r = r1; r <= r2; r++) total += buf_len(e->buf, r) + 1;
        total--;
        char *y = malloc((size_t)(total + 1));
        int pos = 0;
        for (int r = r1; r <= r2; r++) {
            memcpy(y + pos, buf_line(e->buf, r), (size_t)buf_len(e->buf, r));
            pos += buf_len(e->buf, r);
            if (r < r2) { y[pos] = '\n'; pos++; }
        }
        y[pos] = '\0';
        e->regs[0].text = y;
        e->regs[0].len = total;
        e->regs[0].linewise = true;
        e->row = sr; e->col = sc;
    } else {
        int r1, c1, r2, c2;
        if (sr < er || (sr == er && sc <= ec)) {
            r1 = sr; c1 = sc; r2 = er; c2 = ec;
        } else {
            r1 = er; c1 = ec; r2 = sr; c2 = sc;
        }
        if (motion == 'e' || motion == 'E') c2++;
        int total = 0;
        if (r1 == r2) {
            total = c2 - c1;
        } else {
            total = buf_len(e->buf, r1) - c1 + c2;
            for (int r = r1 + 1; r < r2; r++) total += buf_len(e->buf, r);
        }
        char *y = malloc((size_t)(total + 1));
        int pos = 0;
        if (r1 == r2) {
            memcpy(y, buf_line(e->buf, r1) + c1, (size_t)total);
        } else {
            memcpy(y + pos, buf_line(e->buf, r1) + c1, (size_t)(buf_len(e->buf, r1) - c1));
            pos += buf_len(e->buf, r1) - c1;
            for (int r = r1 + 1; r < r2; r++) {
                memcpy(y + pos, buf_line(e->buf, r), (size_t)buf_len(e->buf, r));
                pos += buf_len(e->buf, r);
            }
            memcpy(y + pos, buf_line(e->buf, r2), (size_t)c2);
            pos += c2;
        }
        y[total] = '\0';
        e->regs[0].text = y;
        e->regs[0].len = total;
        e->regs[0].linewise = false;
        e->row = sr; e->col = sc;
    }
    e->goal_col = e->col;
}

void op_shift_right(Editor *e, int count, int motion)
{
    int sr = e->row;
    do_motion(e, motion, count);
    int er = e->row;
    int r1 = min_i(sr, er);
    int r2 = max_i(sr, er);
    for (int r = r1; r <= r2; r++) {
        for (int i = 0; i < e->shiftwidth; i++) {
            buf_insert_char(e->buf, r, 0, ' ');
            undo_push(e, 1, r, i, " ", 1);
        }
    }
    e->row = sr;
    e->goal_col = e->col;
}

void op_shift_left(Editor *e, int count, int motion)
{
    int sr = e->row;
    do_motion(e, motion, count);
    int er = e->row;
    int r1 = min_i(sr, er);
    int r2 = max_i(sr, er);
    for (int r = r1; r <= r2; r++) {
        char *line = buf_line(e->buf, r);
        int sw = min_i(e->shiftwidth, buf_len(e->buf, r));
        int spaces = 0;
        while (spaces < sw && line[spaces] == ' ') spaces++;
        for (int i = 0; i < spaces; i++) {
            buf_delete_char(e->buf, r, 0);
            undo_push(e, 4, r, 0, " ", 1);
        }
    }
    e->row = sr;
    e->goal_col = e->col;
}

void change_repeat(Editor *e)
{
    if (e->last_op == 'd') {
        op_delete(e, e->last_count, e->last_motion);
    } else if (e->last_op == 'c') {
        op_change(e, e->last_count, e->last_motion);
    } else if (e->last_op == 'y') {
        op_yank(e, e->last_count, e->last_motion);
    }
}
