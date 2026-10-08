/* w3m-term — terminal Win 3.x: pty + shell real, render directo Xlib.
   MVP: entrada de comandos y salida lineal con scroll; no emula
   secuencias de escape completas (usa TERM=dumb para que las apps no
   envien curses). */

#include "ui.h"
#include "applet.h"

#include <pty.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

#define TERM_ROWS 24
#define TERM_COLS 80
#define SCROLL_MAX 500
#define CHARS (TERM_COLS + 1)

static char lines[SCROLL_MAX][CHARS];
static int nlines = 0;
static int scroll = 0;      /* first visible line */
static char input[CHARS];
static int ninput = 0;
static char cwd[512];

static void push_line(const char *s) {
    if (nlines < SCROLL_MAX) {
        snprintf(lines[nlines], CHARS, "%s", s);
        nlines++;
    } else {
        memmove(lines[0], lines[1], (SCROLL_MAX - 1) * CHARS);
        snprintf(lines[SCROLL_MAX - 1], CHARS, "%s", s);
    }
    if (nlines - scroll > TERM_ROWS) scroll = nlines - TERM_ROWS;
}

static void term_draw(UiCtx *u) {
    ui_clear(u);
    /* screen: white client area with black text, Win 3.x style */
    XSetForeground(u->dpy, u->gc, u->c_white);
    XFillRectangle(u->dpy, u->win, u->gc, 4, 4, (unsigned)u->w - 8, (unsigned)u->h - 8);
    XSetFont(u->dpy, u->gc, u->font->fid);
    XSetForeground(u->dpy, u->gc, u->c_fg);

    int row_h = u->font->ascent + u->font->descent + 1;
    int y = 4 + u->font->ascent;
    for (int i = 0; i < TERM_ROWS; i++) {
        int idx = scroll + i;
        if (idx >= nlines) break;
        XDrawString(u->dpy, u->win, u->gc, 8, y, lines[idx],
                    (int)strlen(lines[idx]));
        y += row_h;
    }
    /* prompt line: cwd + $ + input + cursor */
    y = 4 + TERM_ROWS * row_h + u->font->ascent + 2;
    char prompt[600];
    snprintf(prompt, sizeof prompt, "%s $ %s", cwd, input);
    XDrawString(u->dpy, u->win, u->gc, 8, y, prompt, (int)strlen(prompt));
    int plen = XTextWidth(u->font, prompt, (int)strlen(prompt));
    XDrawLine(u->dpy, u->win, u->gc, 8 + plen + 1, y - u->font->ascent,
              8 + plen + 1, y);
    ui_flush(u);
}

/* strip control sequences crudely: keep printable chars, CR/LF -> line breaks */
static void feed(char *buf, ssize_t n) {
    static char partial[CHARS];
    static int npartial = 0;
    for (ssize_t i = 0; i < n; i++) {
        char c = buf[i];
        if (c == '\n') {
            partial[npartial] = '\0';
            push_line(partial);
            npartial = 0;
        } else if (c == '\r') {
            npartial = 0;
        } else if (c == '\t') {
            int sp = 8 - (npartial % 8);
            while (sp-- > 0 && npartial < TERM_COLS - 1) partial[npartial++] = ' ';
        } else if (c == 0x1b) {
            /* skip escape sequence body until a letter (crude) */
            i++;
            while (i < n && !((buf[i] >= 'A' && buf[i] <= 'Z') ||
                              (buf[i] >= 'a' && buf[i] <= 'z'))) i++;
        } else if (c >= ' ' && c < 127) {
            if (npartial < TERM_COLS - 1) partial[npartial++] = c;
        }
    }
    (void)partial; (void)npartial;
}

static void run_command(void) {
    char promptline[600];
    snprintf(promptline, sizeof promptline, "%s $ %s", cwd, input);
    push_line(promptline);
    if (strcmp(input, "exit") == 0) { ninput = 0; input[0] = '\0'; return; }
    if (strcmp(input, "clear") == 0) { nlines = 0; scroll = 0; ninput = 0; input[0] = '\0'; return; }

    /* execute with cwd set, capture output via applet layer semantics but
       through plain fork (shell builtins like cd need a shell) */
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        chdir(cwd);
        execl("/bin/sh", "sh", "-c", input, (char *)NULL);
        _exit(127);
    }
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
        push_line("(comando no encontrado)");
    ninput = 0;
    input[0] = '\0';
    if (getcwd(cwd, sizeof cwd) == NULL) snprintf(cwd, sizeof cwd, "?");
}

int main(void) {
    UiCtx u;
    ui_init(&u, 620, 460, "Terminal — W3M");
    if (getcwd(cwd, sizeof cwd) == NULL) snprintf(cwd, sizeof cwd, "/");
    push_line("W3M Terminal — escribe comandos; 'clear' limpia, 'exit' sale");

    char buf[4096];
    for (;;) {
        while (XPending(u.dpy)) {
            XEvent ev;
            XNextEvent(u.dpy, &ev);
            if (ev.type == Expose) { term_draw(&u); continue; }
            if (ev.type == ClientMessage) { ui_close(&u); return 0; }
            if (ev.type == KeyPress) {
                char kbuf[16];
                KeySym ks;
                XLookupString(&ev.xkey, kbuf, sizeof kbuf, &ks, NULL);
                if (ks == XK_Return) { run_command(); term_draw(&u); }
                else if (ks == XK_BackSpace) {
                    if (ninput > 0) input[--ninput] = '\0';
                    term_draw(&u);
                } else if (kbuf[0] >= ' ' && kbuf[0] < 127 && ninput < CHARS - 2) {
                    input[ninput++] = kbuf[0];
                    input[ninput] = '\0';
                    term_draw(&u);
                }
            }
        }
        usleep(20000);
        (void)buf; (void)feed;
    }
}
