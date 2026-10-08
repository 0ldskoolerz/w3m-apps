/* w3m-notepad — editor simple estilo Notepad: una línea de estado,
   edición línea a línea, abrir/guardar archivos de texto plano.
   MVP de texto: buffer de líneas, cursor, guardado con busybox-less
   (fwrite directo). */

#include "ui.h"
#include "applet.h"

#include <sys/stat.h>

#define MAX_LINES 512
#define LINE_LEN 256
#define VISIBLE_ROWS 22

static char lines[MAX_LINES][LINE_LEN];
static int nlines = 1;
static int cx = 0, cy = 0;          /* cursor col/line */
static int top = 0;
static char path[512] = "";
static char status[128] = "sin título";

static void load_file(const char *p) {
    FILE *f = fopen(p, "r");
    if (!f) { snprintf(status, sizeof status, "no se pudo abrir %s", p); return; }
    nlines = 0;
    char buf[LINE_LEN];
    while (nlines < MAX_LINES && fgets(buf, sizeof buf, f)) {
        buf[strcspn(buf, "\r\n")] = '\0';
        snprintf(lines[nlines], LINE_LEN, "%s", buf);
        nlines++;
    }
    if (nlines == 0) { lines[0][0] = '\0'; nlines = 1; }
    fclose(f);
    snprintf(path, sizeof path, "%s", p);
    snprintf(status, sizeof status, "%s (%d líneas)", p, nlines);
    cx = cy = top = 0;
}

static void save_file(void) {
    if (!path[0]) { snprintf(status, sizeof status, "usa 'Guardar como' con un nombre"); return; }
    FILE *f = fopen(path, "w");
    if (!f) { snprintf(status, sizeof status, "no se pudo guardar"); return; }
    for (int i = 0; i < nlines; i++) fprintf(f, "%s\n", lines[i]);
    fclose(f);
    snprintf(status, sizeof status, "guardado: %s", path);
}

static void insert_char(char c) {
    if (cy >= MAX_LINES) return;
    char *line = lines[cy];
    int len = (int)strlen(line);
    if (len >= LINE_LEN - 2) return;
    memmove(line + cx + 1, line + cx, (size_t)(len - cx));
    line[cx] = c;
    line[len + 1] = '\0';
    cx++;
}

static void newline(void) {
    if (nlines >= MAX_LINES) return;
    memmove(lines + cy + 2, lines + cy + 1,
            (size_t)(nlines - cy - 1) * LINE_LEN);
    /* split current line at cx */
    char *line = lines[cy];
    snprintf(lines[cy + 1], LINE_LEN, "%s", line + cx);
    line[cx] = '\0';
    nlines++;
    cy++; cx = 0;
    if (cy - top >= VISIBLE_ROWS) top = cy - VISIBLE_ROWS + 1;
}

static void backspace(void) {
    if (cx > 0) {
        char *line = lines[cy];
        int len = (int)strlen(line);
        memmove(line + cx - 1, line + cx, (size_t)(len - cx + 1));
        cx--;
    } else if (cy > 0) {
        /* join with previous line */
        int plen = (int)strlen(lines[cy - 1]);
        if (plen + strlen(lines[cy]) < LINE_LEN - 1) {
            strcat(lines[cy - 1], lines[cy]);
            cx = plen;
            memmove(lines + cy, lines + cy + 1,
                    (size_t)(nlines - cy - 1) * LINE_LEN);
            nlines--;
            cy--;
        }
    }
}

static void pad_draw(UiCtx *u) {
    ui_clear(u);
    /* menu bar: Guardar | Abrir ruta (lee ~/nota.txt por defecto) */
    ui_button(u, 4, 4, 80, 22, "Guardar", false);
    ui_button(u, 88, 4, 80, 22, "Abrir ~/w3m.txt", false);
    /* text area */
    XSetForeground(u->dpy, u->gc, u->c_white);
    XFillRectangle(u->dpy, u->win, u->gc, 4, 30, (unsigned)u->w - 8, (unsigned)u->h - 60);
    XSetForeground(u->dpy, u->gc, u->c_fg);
    int row_h = u->font->ascent + u->font->descent + 1;
    int y = 30 + u->font->ascent + 2;
    for (int i = 0; i < VISIBLE_ROWS; i++) {
        int idx = top + i;
        if (idx >= nlines) break;
        XDrawString(u->dpy, u->win, u->gc, 8, y, lines[idx],
                    (int)strlen(lines[idx]));
        y += row_h;
    }
    /* cursor block */
    int cury = 30 + (cy - top) * row_h + 2;
    int cxpix = 8 + XTextWidth(u->font, lines[cy], cx);
    XFillRectangle(u->dpy, u->win, u->gc, cxpix, cury, 7, row_h);
    /* status */
    ui_bevel(u, 4, u->h - 26, u->w - 8, 20, true);
    ui_text(u, 8, u->h - 12, status);
    ui_flush(u);
}

int main(int argc, char **argv) {
    UiCtx u;
    ui_init(&u, 640, 420, "Bloc de notas — W3M");
    lines[0][0] = '\0';
    if (argc > 1) load_file(argv[1]);

    XEvent ev;
    for (;;) {
        if (!ui_next_event(&u, &ev)) break;
        if (ev.type == Expose) { pad_draw(&u); continue; }
        if (ev.type == ButtonPress && ev.xbutton.y < 28) {
            if (ev.xbutton.x < 84) save_file();
            else if (ev.xbutton.x < 170) load_file(getenv("HOME") && getenv("HOME")[0]
                        ? (snprintf(path, sizeof path, "%s/w3m.txt", getenv("HOME")), path)
                        : "w3m.txt");
            pad_draw(&u);
            continue;
        }
        if (ev.type == KeyPress) {
            char kb[8]; KeySym ks;
            int n = XLookupString(&ev.xkey, kb, sizeof kb, &ks, NULL);
            if (ks == XK_Return) newline();
            else if (ks == XK_BackSpace) backspace();
            else if (ks == XK_Left && cx > 0) cx--;
            else if (ks == XK_Right && cx < (int)strlen(lines[cy])) cx++;
            else if (ks == XK_Up && cy > 0) { cy--; if (cy < top) top = cy;
                if (cx > (int)strlen(lines[cy])) cx = (int)strlen(lines[cy]); }
            else if (ks == XK_Down && cy < nlines - 1) { cy++;
                if (cy - top >= VISIBLE_ROWS) top = cy - VISIBLE_ROWS + 1;
                if (cx > (int)strlen(lines[cy])) cx = (int)strlen(lines[cy]); }
            else if (n == 1 && kb[0] >= ' ' && kb[0] < 127) insert_char(kb[0]);
            pad_draw(&u);
        }
    }
    ui_close(&u);
    return 0;
}
