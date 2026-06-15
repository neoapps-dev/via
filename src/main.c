#include "editor.h"
#include "config.h"
int main(int argc, char *argv[])
{
    Editor *e = ed_new();
    config_load(e);
    ed_msg_clear(e);
    if (argc > 1) {
        ed_open(e, argv[1]);
    }
    ed_run(e);
    ed_free(e);
    return 0;
}
