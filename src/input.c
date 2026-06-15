#include <curses.h>
#include "input.h"
int input_read(Editor *e)
{
    (void)e;
    int ch = getch();
    return ch;
}

void input_pushback(int ch)
{
    ungetch(ch);
}
