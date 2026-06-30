#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include "ex.h"
#include "motion.h"
void ex_handle_key(Editor *e, int key)
{
    if (key == 27) {
        e->cmd_active = false;
        memset(e->cmd_buf, 0, VIA_CMD_MAX + 1);
        e->cmd_len = 0;
        e->cmd_pos = 0;
        return;
    }
    if (key == '\n' || key == '\r' || key == KEY_ENTER) {
        e->cmd_active = false;
        if (e->cmd_len > 0 && e->cmd_buf[0] == '/') {
            memmove(e->cmd_buf, e->cmd_buf + 1, (size_t)(e->cmd_len));
            e->cmd_len--;
            e->cmd_buf[e->cmd_len] = '\0';
            strncpy(e->last_search, e->cmd_buf, VIA_CMD_MAX);
            e->search_backward = false;
            ex_cmd_number(e, e->last_search);
        } else if (e->cmd_len > 0 && e->cmd_buf[0] == '?') {
            memmove(e->cmd_buf, e->cmd_buf + 1, (size_t)(e->cmd_len));
            e->cmd_len--;
            e->cmd_buf[e->cmd_len] = '\0';
            strncpy(e->last_search, e->cmd_buf, VIA_CMD_MAX);
            e->search_backward = true;
            ex_cmd_number(e, e->last_search);
        } else {
            ex_execute(e, e->cmd_buf);
        }
        memset(e->cmd_buf, 0, VIA_CMD_MAX + 1);
        e->cmd_len = 0;
        e->cmd_pos = 0;
        return;
    }
    if (key == KEY_BACKSPACE || key == 127 || key == 8) {
        if (e->cmd_pos > 0) {
            memmove(e->cmd_buf + e->cmd_pos - 1, e->cmd_buf + e->cmd_pos,
                    (size_t)(e->cmd_len - e->cmd_pos));
            e->cmd_len--;
            e->cmd_pos--;
            e->cmd_buf[e->cmd_len] = '\0';
        }
        return;
    }
    if (key == KEY_LEFT || key == CTRL('B')) {
        if (e->cmd_pos > 0) e->cmd_pos--;
        return;
    }
    if (key == KEY_RIGHT || key == CTRL('F')) {
        if (e->cmd_pos < e->cmd_len) e->cmd_pos++;
        return;
    }
    if (key == KEY_HOME || key == CTRL('A')) {
        e->cmd_pos = 0;
        return;
    }
    if (key == KEY_END || key == CTRL('E')) {
        e->cmd_pos = e->cmd_len;
        return;
    }
    if (key >= 32 && key <= 126) {
        if (e->cmd_len < VIA_CMD_MAX) {
            memmove(e->cmd_buf + e->cmd_pos + 1, e->cmd_buf + e->cmd_pos,
                    (size_t)(e->cmd_len - e->cmd_pos + 1));
            e->cmd_buf[e->cmd_pos] = (char)key;
            e->cmd_len++;
            e->cmd_pos++;
        }
        return;
    }
}

void ex_cmd_write(Editor *e, const char *arg)
{
    const char *path = arg;
    if (*path == '\0') path = e->filename;
    if (!path || *path == '\0') {
        ed_msg(e, "No file name");
        return;
    }
    if (buf_save(e->buf, path) == 0) {
        if (path != e->filename) {
            if (e->filename) free(e->filename);
            e->filename = strdup(path);
        }
        e->modified = false;
        e->undo_save_pos = e->undo->pos;
        ed_msg(e, "\"%s\" written", path);
    } else {
        ed_msg(e, "Can't write file");
    }
}

void ex_cmd_quit(Editor *e, const char *arg)
{
    (void)arg;
    if (e->modified) {
        ed_msg(e, "No write since last change (use :q! to discard)");
        return;
    }
    e->running = false;
}

void ex_cmd_wq(Editor *e, const char *arg)
{
    ex_cmd_write(e, arg);
    if (!e->modified) e->running = false;
}

void ex_cmd_edit(Editor *e, const char *arg)
{
    if (!arg || *arg == '\0') {
        ed_msg(e, "No file name");
        return;
    }
    ed_open(e, arg);
    e->modified = false;
    e->undo_save_pos = e->undo->pos;
}

void ex_cmd_set(Editor *e, const char *arg)
{
    if (!arg || *arg == '\0') {
        ed_msg(e, "number=%s list=%s syntax=%s tabstop=%d shiftwidth=%d scrolloff=%d",
               e->opts.number ? "true" : "false",
               e->opts.list ? "true" : "false",
               e->opts.syntax ? "true" : "false",
               e->opts.tabstop, e->opts.shiftwidth, e->opts.scrolloff);
        return;
    }
    char name[64];
    char value[64];
    const char *eq = strchr(arg, '=');
    if (eq) {
        size_t nlen = (size_t)(eq - arg);
        if (nlen >= sizeof(name)) nlen = sizeof(name) - 1;
        memcpy(name, arg, nlen); name[nlen] = '\0';
        strncpy(value, eq + 1, sizeof(value) - 1); value[sizeof(value) - 1] = '\0';
    } else {
        strncpy(name, arg, sizeof(name) - 1); name[sizeof(name) - 1] = '\0';
        value[0] = '\0';
    }
    char *end;
    long n = strtol(value, &end, 10);
    if (strcmp(name, "tabstop") == 0 || strcmp(name, "ts") == 0) {
        if (*end == '\0') { e->tabstop = (int)n; via_opts.tabstop = (int)n; }
    } else if (strcmp(name, "shiftwidth") == 0 || strcmp(name, "sw") == 0) {
        if (*end == '\0') { e->shiftwidth = (int)n; via_opts.shiftwidth = (int)n; }
    } else if (strcmp(name, "scrolloff") == 0 || strcmp(name, "so") == 0) {
        if (*end == '\0') { e->scrolloff = (int)n; via_opts.scrolloff = (int)n; }
    } else if (strcmp(name, "number") == 0 || strcmp(name, "nu") == 0) {
        e->show_number = !e->show_number; via_opts.number = e->show_number;
    } else if (strcmp(name, "nonumber") == 0 || strcmp(name, "nonu") == 0) {
        e->show_number = false; via_opts.number = false;
    } else if (strcmp(name, "list") == 0) {
        e->show_list = !e->show_list; via_opts.list = e->show_list;
    } else if (strcmp(name, "nolist") == 0) {
        e->show_list = false; via_opts.list = false;
    } else if (strcmp(name, "syntax") == 0) {
        e->opts.syntax = !e->opts.syntax; via_opts.syntax = e->opts.syntax;
    } else if (strcmp(name, "nosyntax") == 0) {
        e->opts.syntax = false; via_opts.syntax = false;
    } else {
        ed_msg(e, "Unknown option: %s", name);
    }
}

void ex_cmd_substitute(Editor *e, const char *arg)
{
    (void)e;
    (void)arg;
    ed_msg(e, "Substitute not yet implemented");
}

static void search_forward(Editor *e, const char *pattern)
{
    int plen = (int)strlen(pattern);
    for (int r = e->row; r < buf_count(e->buf); r++) {
        char *line = buf_line(e->buf, r);
        int start = (r == e->row) ? e->col + 1 : 0;
        int llen = buf_len(e->buf, r);
        for (int c = start; c <= llen - plen; c++) {
            if (strncmp(line + c, pattern, (size_t)plen) == 0) {
                e->row = r; e->col = c; e->goal_col = c;
                ed_msg(e, "Search: %s", pattern);
                return;
            }
        }
    }
    for (int r = 0; r <= e->row; r++) {
        char *line = buf_line(e->buf, r);
        int end = (r == e->row) ? e->col : buf_len(e->buf, r);
        for (int c = 0; c <= end - plen; c++) {
            if (strncmp(line + c, pattern, (size_t)plen) == 0) {
                e->row = r; e->col = c; e->goal_col = c;
                ed_msg(e, "Search: %s (wrapped)", pattern);
                return;
            }
        }
    }
    ed_msg(e, "Pattern not found: %s", pattern);
}

static void search_backward(Editor *e, const char *pattern)
{
    int plen = (int)strlen(pattern);
    for (int r = e->row; r >= 0; r--) {
        char *line = buf_line(e->buf, r);
        int start = (r == e->row) ? e->col - 1 : buf_len(e->buf, r) - 1;
        int llen = buf_len(e->buf, r);
        for (int c = start; c >= 0; c--) {
            if (c + plen <= llen && strncmp(line + c, pattern, (size_t)plen) == 0) {
                e->row = r; e->col = c; e->goal_col = c;
                ed_msg(e, "Search: %s", pattern);
                return;
            }
        }
    }
    for (int r = buf_count(e->buf) - 1; r > e->row; r--) {
        char *line = buf_line(e->buf, r);
        int llen = buf_len(e->buf, r);
        for (int c = llen - plen; c >= 0; c--) {
            if (strncmp(line + c, pattern, (size_t)plen) == 0) {
                e->row = r; e->col = c; e->goal_col = c;
                ed_msg(e, "Search: %s (wrapped)", pattern);
                return;
            }
        }
    }
    ed_msg(e, "Pattern not found: %s", pattern);
}

void ex_cmd_number(Editor *e, const char *arg)
{
    if (!arg || *arg == '\0') return;
    if (e->search_backward) search_backward(e, arg);
    else search_forward(e, arg);
}

static int elem_from_name(const char *name)
{
    static const char *names[VIA_ELEMS] = {
        "normal", "insert", "visual", "status",
        "status-insert", "status-visual", "cmdline",
        "linenum", "cursor-line", "selection",
        "search", "eol",
        "keyword", "string", "comment", "number",
        "type", "preproc", "operator"
    };
    for (int i = 0; i < VIA_ELEMS; i++)
        if (strcmp(name, names[i]) == 0) return i;
    return -1;
}

void ex_cmd_highlight(Editor *e, const char *arg)
{
    while (*arg == ' ') arg++;
    if (*arg == '\0') {
        ed_msg(e, "Usage: highlight <elem> fg=<c> bg=<c> [attr=<a>]");
        return;
    }
    char elem[32];
    int i = 0;
    while (*arg && *arg != ' ' && i < (int)sizeof(elem) - 1) elem[i++] = *arg++;
    elem[i] = '\0';
    int ei = elem_from_name(elem);
    if (ei < 0) {
        ed_msg(e, "Unknown element: %s", elem);
        return;
    }
    const char *fg = NULL, *bg = NULL, *attr = NULL;
    while (*arg) {
        while (*arg == ' ') arg++;
        if (*arg == '\0') break;
        if (strncmp(arg, "fg=", 3) == 0) { fg = arg + 3; arg += 3; }
        else if (strncmp(arg, "bg=", 3) == 0) { bg = arg + 3; arg += 3; }
        else if (strncmp(arg, "attr=", 5) == 0) { attr = arg + 5; arg += 5; }
        else { arg++; continue; }
        while (*arg && *arg != ' ') arg++;
    }
    if (theme_set(ei, fg, bg, attr) == 0) {
        ed_msg(e, "highlight %s set", elem);
    } else {
        ed_msg(e, "Invalid highlight values");
    }
}

void ex_execute(Editor *e, const char *cmd)
{
    while (*cmd == ' ') cmd++;
    if (*cmd == ':') cmd++;
    while (*cmd == ' ') cmd++;
    if (*cmd == '\0') return;
    if (*cmd == '!') {
        e->running = false;
        return;
    }
    if (*cmd == '/' || *cmd == '?') {
        strncpy(e->last_search, cmd + 1, VIA_CMD_MAX);
        e->search_backward = (*cmd == '?');
        ex_cmd_number(e, e->last_search);
        return;
    }
    if (*cmd >= '0' && *cmd <= '9') {
        char *end;
        long n = strtol(cmd, &end, 10);
        if (end && (*end == '\0' || *end == ' ')) {
            motion_goto_line(e, (int)n);
        }
        return;
    }
    char buf[VIA_CMD_MAX + 1];
    strncpy(buf, cmd, VIA_CMD_MAX);
    buf[VIA_CMD_MAX] = '\0';
    char *arg = buf;
    while (*arg && *arg != ' ') arg++;
    if (*arg == ' ') {
        *arg = '\0';
        arg++;
        while (*arg == ' ') arg++;
    } else {
        arg = "";
    }
    if (strcmp(buf, "w") == 0) ex_cmd_write(e, arg);
    else if (strcmp(buf, "write") == 0) ex_cmd_write(e, arg);
    else if (strcmp(buf, "q") == 0) ex_cmd_quit(e, arg);
    else if (strcmp(buf, "quit") == 0) ex_cmd_quit(e, arg);
    else if (strcmp(buf, "q!") == 0) { e->modified = false; e->running = false; }
    else if (strcmp(buf, "wq") == 0) ex_cmd_wq(e, arg);
    else if (strcmp(buf, "e") == 0) ex_cmd_edit(e, arg);
    else if (strcmp(buf, "edit") == 0) ex_cmd_edit(e, arg);
    else if (strcmp(buf, "set") == 0) ex_cmd_set(e, arg);
    else if (strcmp(buf, "s") == 0) ex_cmd_substitute(e, arg);
    else if (strcmp(buf, "x") == 0) { ex_cmd_wq(e, arg); }
    else if (strcmp(buf, "n") == 0) { ex_cmd_number(e, e->last_search); }
    else if (strcmp(buf, "highlight") == 0 || strcmp(buf, "hi") == 0) { ex_cmd_highlight(e, arg); }
    else ed_msg(e, "Unknown command: %s", buf);
}
