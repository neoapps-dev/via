#include <string.h>
#include <stdio.h>
#include <curses.h>
#include "render.h"
#include "theme.h"
static int scr_rows;
static int scr_cols;
void render_init(void)
{
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    set_escdelay(0);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);
    printf("\033[?1002h\033[?1006h"); fflush(stdout);
    curs_set(1);
    scr_rows = LINES;
    scr_cols = COLS;
    theme_init();
    theme_default();
}

void render_end(void)
{
    printf("\033[?1006l\033[?1002l"); fflush(stdout);
    endwin();
}

void render_resize(void)
{
    scr_rows = LINES;
    scr_cols = COLS;
}

static void render_line(Editor *e, int screen_row, int buf_row)
{
    if (buf_row < 0 || buf_row >= buf_count(e->buf)) {
        move(screen_row, 0);
        clrtoeol();
        if (buf_row == buf_count(e->buf)) {
            attrset(theme_attr(VIA_EOL) | theme_pair(VIA_EOL));
            mvaddch(screen_row, 0, '~');
        }
        return;
    }
    char *line = buf_line(e->buf, buf_row);
    int len = buf_len(e->buf, buf_row);
    int show_num = e->show_number;
    int num_width = show_num ? 7 : 0;
    if (show_num) {
        attrset(theme_attr(VIA_LINENUM) | theme_pair(VIA_LINENUM));
        char num[16];
        snprintf(num, sizeof(num), "%*d ", 5, buf_row + 1);
        mvaddstr(screen_row, 0, num);
    }
    bool cursor_line = (buf_row == e->row && e->mode != MODE_INSERT);
    bool in_visual = false;
    if ((e->mode == MODE_VISUAL || e->mode == MODE_VISUAL_LINE) && e->visual.active) {
        int v1 = e->visual.start_row;
        int v2 = e->row;
        if (v1 > v2) { int t = v1; v1 = v2; v2 = t; }
        in_visual = (buf_row >= v1 && buf_row <= v2);
    }
    int pair;
    int attr;
    if (in_visual) {
        pair = theme_pair(VIA_SELECTION);
        attr = theme_attr(VIA_SELECTION);
    } else if (cursor_line) {
        pair = theme_pair(VIA_CURSOR_LINE);
        attr = theme_attr(VIA_CURSOR_LINE);
    } else {
        pair = theme_pair(VIA_NORMAL);
        attr = theme_attr(VIA_NORMAL);
    }
    attrset(attr | pair);
    int x = num_width;
    for (int i = 0; i < len && x < scr_cols; i++) {
        if (line[i] == '\t') {
            int stop = e->tabstop - (x - num_width) % e->tabstop;
            for (int j = 0; j < stop && x < scr_cols; j++) {
                mvaddch(screen_row, x, ' ');
                x++;
            }
        } else if (line[i] < 32) {
            mvaddch(screen_row, x, '^');
            x++;
            if (x < scr_cols) {
                mvaddch(screen_row, x, line[i] + 64);
                x++;
            }
        } else {
            mvaddch(screen_row, x, line[i]);
            x++;
        }
    }
    move(screen_row, x);
    clrtoeol();
}

void render_status(Editor *e)
{
    int r = scr_rows - 1;
    int c = scr_cols;
    if (e->cmd_active) {
        attrset(theme_attr(VIA_CMDLINE) | theme_pair(VIA_CMDLINE));
        move(r, 0);
        clrtoeol();
        mvaddnstr(r, 0, e->cmd_buf, e->cmd_len);
        move(r, e->cmd_pos);
        return;
    }

    if (e->msg[0]) {
        attrset(theme_attr(VIA_CMDLINE) | theme_pair(VIA_CMDLINE));
        move(r, 0);
        clrtoeol();
        mvaddstr(r, 0, e->msg);
        return;
    }

    const char *mode_str = "";
    int mode_pair = theme_pair(VIA_STATUS);
    int mode_attr = theme_attr(VIA_STATUS);
    switch (e->mode) {
    case MODE_NORMAL:
        mode_str = " NORMAL ";
        mode_pair = theme_pair(VIA_STATUS);
        mode_attr = theme_attr(VIA_STATUS);
        break;
    case MODE_INSERT:
        mode_str = " INSERT ";
        mode_pair = theme_pair(VIA_STATUS_INSERT);
        mode_attr = theme_attr(VIA_STATUS_INSERT);
        break;
    case MODE_VISUAL:
    case MODE_VISUAL_LINE:
        mode_str = " VISUAL ";
        mode_pair = theme_pair(VIA_STATUS_VISUAL);
        mode_attr = theme_attr(VIA_STATUS_VISUAL);
        break;
    case MODE_VISUAL_BLOCK:
        mode_str = " VIS-BLOCK ";
        mode_pair = theme_pair(VIA_STATUS_VISUAL);
        mode_attr = theme_attr(VIA_STATUS_VISUAL);
        break;
    case MODE_REPLACE:
        mode_str = " REPLACE ";
        mode_pair = theme_pair(VIA_STATUS_INSERT);
        mode_attr = theme_attr(VIA_STATUS_INSERT);
        break;
    default:
        mode_str = "";
        break;
    }

    attrset(mode_attr | mode_pair);
    for (int i = 0; i < c; i++)
        mvaddch(r, i, ' ');
    int x = 0;
    mvaddstr(r, x, mode_str);
    x += (int)strlen(mode_str);
    if (e->filename) {
        mvaddstr(r, x, e->filename);
        x += (int)strlen(e->filename);
    } else {
        const char *no = "[No Name]";
        mvaddstr(r, x, no);
        x += (int)strlen(no);
    }
    if (e->modified) {
        mvaddstr(r, x, "  [+]" );
        x += 5;
    }

    char right[64];
    int total = buf_count(e->buf);
    int pct = total > 0 ? (int)((long)(e->row + 1) * 100 / total) : 0;
    snprintf(right, sizeof(right), " %d,%d  %d%% ", e->row + 1, e->col + 1, pct);
    int rx = c - (int)strlen(right);
    mvaddstr(r, rx, right);
}

void render_cmdline(Editor *e)
{
    (void)e;
}

void render_draw(Editor *e)
{
    scr_rows = LINES;
    scr_cols = COLS;
    ed_scroll(e);
    int text_rows = scr_rows - 1;
    for (int i = 0; i < text_rows; i++) {
        int buf_row = e->top_row + i;
        render_line(e, i, buf_row);
    }
    render_status(e);
    int cursor_row = e->row - e->top_row;
    int cursor_col = e->col - e->left_col;
    int num_width = e->show_number ? 7 : 0;
    cursor_col += num_width;
    if (cursor_row >= 0 && cursor_row < text_rows && cursor_col >= 0 && cursor_col < scr_cols) {
        move(cursor_row, cursor_col);
    }
    curs_set((e->mode == MODE_INSERT) ? 1 : 2);
    refresh();
}

void render_refresh(void)
{
    refresh();
}

int ed_rows(void) { return LINES; }
int ed_cols(void) { return COLS; }
