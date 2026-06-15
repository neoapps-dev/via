#ifndef VIA_RENDER_H
#define VIA_RENDER_H
#include "editor.h"
void render_init(void);
void render_end(void);
void render_draw(Editor *e);
void render_status(Editor *e);
void render_cmdline(Editor *e);
void render_refresh(void);
void render_resize(void);
#endif
