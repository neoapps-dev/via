CC ?= gcc
override CFLAGS += -std=c11 -Wall -Wextra -pedantic -g -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE $(shell pkg-config --cflags ncurses 2>/dev/null)
LDFLAGS = $(shell pkg-config --libs ncurses 2>/dev/null || echo '-lncurses -ltinfo')
SRCDIR = src
OBJDIR = obj
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
HEADERS = $(wildcard $(SRCDIR)/*.h)
.PHONY: all clean
all: via
via: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(SRCDIR) -c -o $@ $<

$(OBJDIR):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR) via
