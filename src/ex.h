#ifndef VIA_EX_H
#define VIA_EX_H
#include "editor.h"
void ex_execute(Editor *e, const char *cmd);
void ex_handle_key(Editor *e, int key);
void ex_cmd_write(Editor *e, const char *arg);
void ex_cmd_quit(Editor *e, const char *arg);
void ex_cmd_wq(Editor *e, const char *arg);
void ex_cmd_edit(Editor *e, const char *arg);
void ex_cmd_set(Editor *e, const char *arg);
void ex_cmd_substitute(Editor *e, const char *arg);
void ex_cmd_number(Editor *e, const char *arg);
#endif
