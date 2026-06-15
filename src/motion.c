#include <ctype.h>
#include <curses.h>
#include "motion.h"
void motion_left(Editor *e, int n)
{
    for (int i = 0; i < n; i++) {
        if (e->col > 0) e->col--;
    }
    e->goal_col = e->col;
}

void motion_right(Editor *e, int n)
{
    int max = buf_len(e->buf, e->row);
    for (int i = 0; i < n; i++) {
        if (e->col < max) e->col++;
    }
    e->goal_col = e->col;
}

void motion_up(Editor *e, int n)
{
    for (int i = 0; i < n; i++) {
        if (e->row > 0) e->row--;
    }
    int max = buf_len(e->buf, e->row);
    if (e->goal_col > max) e->col = max;
    else e->col = e->goal_col;
}

void motion_down(Editor *e, int n)
{
    int last = buf_count(e->buf) - 1;
    for (int i = 0; i < n; i++) {
        if (e->row < last) e->row++;
    }
    int max = buf_len(e->buf, e->row);
    if (e->goal_col > max) e->col = max;
    else e->col = e->goal_col;
}

void motion_bol(Editor *e)
{
    e->col = 0;
    e->goal_col = 0;
}

void motion_eol(Editor *e)
{
    e->col = buf_len(e->buf, e->row);
    e->goal_col = e->col;
}

void motion_first_non_blank(Editor *e)
{
    char *line = buf_line(e->buf, e->row);
    if (!line) { e->col = 0; return; }
    int i = 0;
    while (line[i] == ' ' || line[i] == '\t') i++;
    e->col = i;
    e->goal_col = i;
}

void motion_word_next(Editor *e, int n)
{
    for (int w = 0; w < n; w++) {
        char *line = buf_line(e->buf, e->row);
        int len = buf_len(e->buf, e->row);
        int i = e->col;
        while (i < len && !isalnum((unsigned char)line[i]) && line[i] != '_') i++;
        while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_')) i++;
        while (i < len && !isalnum((unsigned char)line[i]) && line[i] != '_') i++;
        if (i >= len) {
            if (e->row < buf_count(e->buf) - 1) {
                e->row++; e->col = 0;
            }
        } else {
            e->col = i;
        }
    }
    e->goal_col = e->col;
}

void motion_word_prev(Editor *e, int n)
{
    for (int w = 0; w < n; w++) {
        char *line = buf_line(e->buf, e->row);
        int i = e->col;
        if (i <= 0) {
            if (e->row > 0) {
                e->row--;
                e->col = buf_len(e->buf, e->row);
            }
            continue;
        }
        i--;
        if (!isalnum((unsigned char)line[i]) && line[i] != '_') {
            while (i > 0 && !isalnum((unsigned char)line[i]) && line[i] != '_') i--;
        }
        while (i > 0 && (isalnum((unsigned char)line[i]) || line[i] == '_')) i--;
        if (i > 0 || (isalnum((unsigned char)line[0]) || line[0] == '_')) {
            if (i > 0) i++;
            e->col = i;
        } else {
            e->col = 0;
        }
    }
    e->goal_col = e->col;
}

void motion_WORD_next(Editor *e, int n)
{
    for (int w = 0; w < n; w++) {
        char *line = buf_line(e->buf, e->row);
        int len = buf_len(e->buf, e->row);
        int i = e->col;
        while (i < len && line[i] == ' ') i++;
        while (i < len && line[i] != ' ') i++;
        while (i < len && line[i] == ' ') i++;
        if (i >= len) {
            if (e->row < buf_count(e->buf) - 1) {
                e->row++; e->col = 0;
            }
        } else {
            e->col = i;
        }
    }
    e->goal_col = e->col;
}

void motion_WORD_prev(Editor *e, int n)
{
    for (int w = 0; w < n; w++) {
        char *line = buf_line(e->buf, e->row);
        int i = e->col;
        if (i <= 0) {
            if (e->row > 0) {
                e->row--;
                e->col = buf_len(e->buf, e->row);
            }
            continue;
        }
        i--;
        while (i > 0 && line[i] == ' ') i--;
        while (i > 0 && line[i] != ' ') i--;
        if (line[i] == ' ' && i > 0) i++;
        else if (line[i] != ' ') i++;
        e->col = i;
    }
    e->goal_col = e->col;
}

void motion_end_word(Editor *e, int n)
{
    for (int w = 0; w < n; w++) {
        char *line = buf_line(e->buf, e->row);
        int len = buf_len(e->buf, e->row);
        int i = e->col;
        if (i >= len) {
            if (e->row < buf_count(e->buf) - 1) {
                e->row++; e->col = 0; i = 0;
                line = buf_line(e->buf, e->row);
                len = buf_len(e->buf, e->row);
            }
        }
        while (i < len && !isalnum((unsigned char)line[i]) && line[i] != '_') i++;
        while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_')) i++;
        if (i > 0) e->col = i - 1;
        else e->col = 0;
    }
    e->goal_col = e->col;
}

void motion_end_WORD(Editor *e, int n)
{
    for (int w = 0; w < n; w++) {
        char *line = buf_line(e->buf, e->row);
        int len = buf_len(e->buf, e->row);
        int i = e->col;
        if (i >= len) {
            if (e->row < buf_count(e->buf) - 1) {
                e->row++; e->col = 0; i = 0;
                line = buf_line(e->buf, e->row);
                len = buf_len(e->buf, e->row);
            }
        }
        while (i < len && line[i] == ' ') i++;
        while (i < len && line[i] != ' ') i++;
        if (i > 0) e->col = i - 1;
        else e->col = 0;
    }
    e->goal_col = e->col;
}

void motion_goto_line(Editor *e, int n)
{
    if (n < 1) n = 1;
    int last = buf_count(e->buf);
    if (n > last) n = last;
    e->row = n - 1;
    int max = buf_len(e->buf, e->row);
    if (e->col > max) e->col = max;
    e->goal_col = e->col;
}

void motion_goto_first(Editor *e)
{
    motion_goto_line(e, 1);
}

void motion_goto_last(Editor *e)
{
    motion_goto_line(e, buf_count(e->buf));
}

void motion_find_next(Editor *e, char c)
{
    char *line = buf_line(e->buf, e->row);
    int len = buf_len(e->buf, e->row);
    for (int i = e->col + 1; i < len; i++) {
        if (line[i] == c) { e->col = i; break; }
    }
    e->goal_col = e->col;
    e->last_find_char = c;
    e->last_find_fwd = true;
    e->last_find_till = false;
}

void motion_find_prev(Editor *e, char c)
{
    char *line = buf_line(e->buf, e->row);
    for (int i = e->col - 1; i >= 0; i--) {
        if (line[i] == c) { e->col = i; break; }
    }
    e->goal_col = e->col;
    e->last_find_char = c;
    e->last_find_fwd = false;
    e->last_find_till = false;
}

void motion_till_next(Editor *e, char c)
{
    char *line = buf_line(e->buf, e->row);
    int len = buf_len(e->buf, e->row);
    for (int i = e->col + 1; i < len; i++) {
        if (line[i] == c) { e->col = i - 1; break; }
    }
    e->goal_col = e->col;
    e->last_find_char = c;
    e->last_find_fwd = true;
    e->last_find_till = true;
}

void motion_till_prev(Editor *e, char c)
{
    char *line = buf_line(e->buf, e->row);
    for (int i = e->col - 1; i >= 0; i--) {
        if (line[i] == c) { e->col = i + 1; break; }
    }
    e->goal_col = e->col;
    e->last_find_char = c;
    e->last_find_fwd = false;
    e->last_find_till = true;
}

void motion_repeat_find(Editor *e)
{
    if (e->last_find_char) {
        if (e->last_find_till) {
            if (e->last_find_fwd) motion_till_next(e, e->last_find_char);
            else motion_till_prev(e, e->last_find_char);
        } else {
            if (e->last_find_fwd) motion_find_next(e, e->last_find_char);
            else motion_find_prev(e, e->last_find_char);
        }
    }
}

void motion_repeat_find_rev(Editor *e)
{
    if (e->last_find_char) {
        if (e->last_find_till) {
            if (e->last_find_fwd) motion_till_prev(e, e->last_find_char);
            else motion_till_next(e, e->last_find_char);
        } else {
            if (e->last_find_fwd) motion_find_prev(e, e->last_find_char);
            else motion_find_next(e, e->last_find_char);
        }
    }
}

void motion_match(Editor *e)
{
    char *line = buf_line(e->buf, e->row);
    int len = buf_len(e->buf, e->row);
    if (!line || e->col >= len) return;
    char c = line[e->col];
    char open, close;
    int dir;
    if (c == '(') { open = '('; close = ')'; dir = 1; }
    else if (c == ')') { open = '('; close = ')'; dir = -1; }
    else if (c == '[') { open = '['; close = ']'; dir = 1; }
    else if (c == ']') { open = '['; close = ']'; dir = -1; }
    else if (c == '{') { open = '{'; close = '}'; dir = 1; }
    else if (c == '}') { open = '{'; close = '}'; dir = -1; }
    else return;
    int depth = 0;
    int r = e->row, c2 = e->col;
    while (r >= 0 && r < buf_count(e->buf)) {
        char *l = buf_line(e->buf, r);
        int llen = buf_len(e->buf, r);
        int start = (r == e->row) ? c2 : (dir > 0 ? 0 : llen - 1);
        int end = (dir > 0) ? llen : -1;
        int step = dir;
        for (int i = start; i != end; i += step) {
            if (l[i] == open) depth++;
            else if (l[i] == close) depth--;
            if (depth == 0) { e->row = r; e->col = i; e->goal_col = i; return; }
        }
        r += dir;
    }
}

void motion_page_down(Editor *e)
{
    int h = ed_rows() - 1;
    motion_down(e, h);
    e->top_row += h;
    if (e->top_row > buf_count(e->buf) - 1)
        e->top_row = buf_count(e->buf) - 1;
}

void motion_page_up(Editor *e)
{
    int h = ed_rows() - 1;
    motion_up(e, h);
    e->top_row -= h;
    if (e->top_row < 0) e->top_row = 0;
}

void motion_half_down(Editor *e)
{
    int h = (ed_rows() - 1) / 2;
    motion_down(e, h);
    e->top_row += h;
    if (e->top_row > buf_count(e->buf) - 1)
        e->top_row = buf_count(e->buf) - 1;
}

void motion_half_up(Editor *e)
{
    int h = (ed_rows() - 1) / 2;
    motion_up(e, h);
    e->top_row -= h;
    if (e->top_row < 0) e->top_row = 0;
}

void motion_scroll_down(Editor *e)
{
    e->top_row++;
    if (e->top_row > buf_count(e->buf) - 1)
        e->top_row = buf_count(e->buf) - 1;
}

void motion_scroll_up(Editor *e)
{
    e->top_row--;
    if (e->top_row < 0) e->top_row = 0;
}

void motion_screen_top(Editor *e)
{
    e->row = e->top_row;
    e->goal_col = e->col;
    int max = buf_len(e->buf, e->row);
    if (e->col > max) e->col = max;
}

void motion_screen_mid(Editor *e)
{
    int h = (ed_rows() - 1) / 2;
    e->row = e->top_row + h;
    if (e->row >= buf_count(e->buf)) e->row = buf_count(e->buf) - 1;
    int max = buf_len(e->buf, e->row);
    if (e->col > max) e->col = max;
    e->goal_col = e->col;
}

void motion_screen_bot(Editor *e)
{
    int h = ed_rows() - 2;
    e->row = e->top_row + h;
    if (e->row >= buf_count(e->buf)) e->row = buf_count(e->buf) - 1;
    int max = buf_len(e->buf, e->row);
    if (e->col > max) e->col = max;
    e->goal_col = e->col;
}

void motion_goto_mark(Editor *e, char c)
{
    int idx = c - 'a';
    if (idx < 0 || idx >= 26 || !e->mark_set[idx]) return;
    e->row = e->marks[idx][0];
    e->col = e->marks[idx][1];
    e->goal_col = e->col;
}

void motion_goto_mark_line(Editor *e, char c)
{
    motion_goto_mark(e, c);
    motion_first_non_blank(e);
}

int motion_is_linewise(int key)
{
    return key == 'j' || key == KEY_DOWN || key == 'k' || key == KEY_UP ||
           key == 'G' || key == 'J';
}

void do_motion(Editor *e, int key, int count)
{
    if (count < 1) count = 1;
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
    case KEY_NPAGE: motion_page_down(e); break;
    case KEY_PPAGE: motion_page_up(e); break;
    case KEY_HOME: motion_bol(e); break;
    }
}
