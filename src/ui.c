#include <dirent.h>
#include "ui.h"
#include <unistd.h>

static Atom wm_delete;

void ui_init(UiCtx *u, int w, int h, const char *title) {
    memset(u, 0, sizeof *u);
    u->w = w; u->h = h;
    u->dpy = XOpenDisplay(NULL);
    if (!u->dpy) { fprintf(stderr, "%s: sin display X\n", title); exit(1); }
    int scr = DefaultScreen(u->dpy);
    u->win = XCreateSimpleWindow(u->dpy, RootWindow(u->dpy, scr),
                                 40, 40, (unsigned)w, (unsigned)h,
                                 1, BlackPixel(u->dpy, scr),
                                 WhitePixel(u->dpy, scr));
    XStoreName(u->dpy, u->win, title);
    XSelectInput(u->dpy, u->win,
                 ExposureMask | ButtonPressMask | ButtonReleaseMask |
                 KeyPressMask | StructureNotifyMask);
    wm_delete = XInternAtom(u->dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(u->dpy, u->win, &wm_delete, 1);
    XMapWindow(u->dpy, u->win);
    u->gc = XCreateGC(u->dpy, u->win, 0, NULL);
    u->font = XLoadQueryFont(u->dpy, "fixed");
    if (!u->font) { fprintf(stderr, "fuente 'fixed' no disponible\n"); exit(1); }
    XSetFont(u->dpy, u->gc, u->font->fid);

    Colormap cm = DefaultColormap(u->dpy, scr);
    XColor col, exact;
    unsigned long alloc(const char *n, const char *fb) {
        if (XAllocNamedColor(u->dpy, cm, n, &col, &exact)) return col.pixel;
        if (XAllocNamedColor(u->dpy, cm, fb, &col, &exact)) return col.pixel;
        return WhitePixel(u->dpy, scr);
    }
    /* same defaults as config/w3m.conf */
    u->c_bg     = alloc("grey70", "grey70");
    u->c_fg     = alloc("black", "black");
    u->c_accent = alloc("navyblue", "navyblue");
    u->c_white  = alloc("white", "white");
    u->c_gray   = alloc("grey50", "grey50");
}

void ui_close(UiCtx *u) {
    XFreeGC(u->dpy, u->gc);
    XCloseDisplay(u->dpy);
}

bool ui_next_event(UiCtx *u, XEvent *ev) {
    XNextEvent(u->dpy, ev);
    if (ev->type == ClientMessage &&
        (Atom)ev->xclient.data.l[0] == wm_delete)
        return false;
    (void)u;
    return true;
}

void ui_bevel(UiCtx *u, int x, int y, int w, int h, bool raised) {
    XSetForeground(u->dpy, u->gc, u->c_bg);
    XFillRectangle(u->dpy, u->win, u->gc, x, y, (unsigned)w, (unsigned)h);
    XSetForeground(u->dpy, u->gc, raised ? u->c_white : u->c_gray);
    XDrawLine(u->dpy, u->win, u->gc, x, y, x + w - 1, y);
    XDrawLine(u->dpy, u->win, u->gc, x, y, x, y + h - 1);
    XSetForeground(u->dpy, u->gc, raised ? u->c_gray : u->c_white);
    XDrawLine(u->dpy, u->win, u->gc, x + w - 1, y, x + w - 1, y + h - 1);
    XDrawLine(u->dpy, u->win, u->gc, x, y + h - 1, x + w - 1, y + h - 1);
}

void ui_button(UiCtx *u, int x, int y, int w, int h, const char *label,
               bool pressed) {
    ui_bevel(u, x, y, w, h, !pressed);
    int tw = XTextWidth(u->font, label, (int)strlen(label));
    XSetForeground(u->dpy, u->gc, u->c_fg);
    XDrawString(u->dpy, u->win, u->gc, x + (w - tw) / 2,
                y + h / 2 + 4, label, (int)strlen(label));
}

void ui_text(UiCtx *u, int x, int y, const char *s) {
    XSetForeground(u->dpy, u->gc, u->c_fg);
    XDrawString(u->dpy, u->win, u->gc, x, y, s, (int)strlen(s));
}

void ui_clear(UiCtx *u) {
    XSetForeground(u->dpy, u->gc, u->c_bg);
    XFillRectangle(u->dpy, u->win, u->gc, 0, 0, (unsigned)u->w, (unsigned)u->h);
}

void ui_flush(UiCtx *u) { XFlush(u->dpy); }

bool file_exists(const char *path) {
    FILE *f = fopen(path, "r");
    if (f) { fclose(f); return true; }
    return false;
}

bool dir_exists(const char *path) {
    DIR *d = opendir(path);
    if (d) { closedir(d); return true; }
    return false;
}
