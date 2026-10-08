CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra -std=gnu11
LDFLAGS ?=

X11_CFLAGS := $(shell pkg-config --cflags x11 2>/dev/null)
X11_LIBS   := $(shell pkg-config --libs x11 2>/dev/null || echo -lX11)

COMMON := src/ui.c src/applet.c
APPS   := w3m-fm w3m-task w3m-calc w3m-notepad w3m-term

.PHONY: all clean

all: $(APPS)

w3m-fm: src/fm.c $(COMMON) src/ui.h src/applet.h
	$(CC) $(CFLAGS) $(X11_CFLAGS) -o $@ src/fm.c $(COMMON) $(X11_LIBS)

w3m-task: src/task.c $(COMMON) src/ui.h src/applet.h
	$(CC) $(CFLAGS) $(X11_CFLAGS) -o $@ src/task.c $(COMMON) $(X11_LIBS)

w3m-calc: src/calc.c $(COMMON) src/ui.h src/applet.h
	$(CC) $(CFLAGS) $(X11_CFLAGS) -o $@ src/calc.c $(COMMON) $(X11_LIBS)

w3m-notepad: src/notepad.c $(COMMON) src/ui.h src/applet.h
	$(CC) $(CFLAGS) $(X11_CFLAGS) -o $@ src/notepad.c $(COMMON) $(X11_LIBS)

w3m-term: src/term.c $(COMMON) src/ui.h src/applet.h
	$(CC) $(CFLAGS) $(X11_CFLAGS) -o $@ src/term.c $(COMMON) $(X11_LIBS)

clean:
	rm -f $(APPS)
