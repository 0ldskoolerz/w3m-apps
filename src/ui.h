#ifndef APPS_UI_H
#define APPS_UI_H

/* Shared UI helpers for w3m-apps: Win 3.x look inherited from the WM.
   Every app is a standalone Xlib program reusing these primitives. */

#if __has_include(<X11/Xlib.h>)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#else
#include "../tests/x11_stub.h"
#endif
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct UiCtx {
    Display *dpy;
    Window win;
    GC gc;
    XFontStruct *font;
    unsigned long c_bg, c_fg, c_accent, c_white, c_gray;
    int w, h;
} UiCtx;

/* initialize from the same named colors as w3m.conf */
void ui_init(UiCtx *u, int w, int h, const char *title);
void ui_close(UiCtx *u);
bool ui_next_event(UiCtx *u, XEvent *ev);   /* returns false on WM_DELETE */

/* drawing primitives */
void ui_bevel(UiCtx *u, int x, int y, int w, int h, bool raised);
void ui_button(UiCtx *u, int x, int y, int w, int h, const char *label,
              bool pressed);
void ui_text(UiCtx *u, int x, int y, const char *s);
void ui_clear(UiCtx *u);
void ui_flush(UiCtx *u);

/* simple file helpers shared by several apps */
bool file_exists(const char *path);
bool dir_exists(const char *path);

#endif
