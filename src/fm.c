/* w3m-fm — explorador de archivos estilo File Manager Win 3.x.
   Toda la lógica pesada (cp, mv, tar, unzip) la hacen applets busybox
   vía la capa applet (argv directo, sin shell). */

#include "ui.h"
#include "applet.h"

#include <libgen.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define LIST_MAX 512
#define NAME_MAX_LEN 128
#define ROW_H 18
#define TOP_H 60

typedef enum { FM_IDLE, FM_COPY, FM_CUT } ClipMode;

static char cwd[512];
static char entries[LIST_MAX][NAME_MAX_LEN];
static int nentries = 0;
static int sel = 0;
static int top = 0;
static char clipboard[600];
static ClipMode clipmode = FM_IDLE;
static char status[128] = "Listo";

/* ---------- busybox-backed operations ---------- */

static void reload(void) {
    nentries = 0;
    char *out = applet_run2("ls", "-1Ap", cwd);
    if (!out) { snprintf(status, sizeof status, "ls falló"); return; }
    char *save = NULL;
    for (char *tok = strtok_r(out, "\n", &save);
         tok && nentries < LIST_MAX; tok = strtok_r(NULL, "\n", &save)) {
        snprintf(entries[nentries], NAME_MAX_LEN, "%s", tok);
        nentries++;
    }
    free(out);
    sel = 0; top = 0;
}

static char *sel_path(void) {
    static char path[700];
    if (sel < 0 || sel >= nentries) return NULL;
    const char *name = entries[sel];
    snprintf(path, sizeof path, "%s/%s", cwd, name);
    return path;
}

static bool is_dir(const char *path) {
    char *out = applet_run2("ls", "-A1pd", path);
    if (!out) return false;
    bool d = out[strlen(out) - 1] == '/' && out[strlen(out) - 2] != '/';
    /* -d devuelve la ruta con / final solo para dirs */
    free(out);
    return d;
}

static void op_crear_carpeta(void) {
    char name[64] = "NuevaCarpeta";
    /* MVP: nombre fijo; renombrar después si se desea */
    char path[700];
    snprintf(path, sizeof path, "%s/%s", cwd, name);
    int st = applet_status("mkdir", (char *const[]){(char *)path, NULL});
    if (st == 0) snprintf(status, sizeof status, "carpeta creada: %s", name);
    else snprintf(status, sizeof status, "mkdir falló (%d)", st);
    reload();
}

static void op_copy(void) {
    char *p = sel_path();
    if (!p) return;
    snprintf(clipboard, sizeof clipboard, "%s", p);
    clipmode = FM_COPY;
    snprintf(status, sizeof status, "copiado: %s", basename(p));
}

static void op_cut(void) {
    char *p = sel_path();
    if (!p) return;
    snprintf(clipboard, sizeof clipboard, "%s", p);
    clipmode = FM_CUT;
    snprintf(status, sizeof status, "cortado: %s", basename(p));
}

static void op_paste(void) {
    if (clipmode == FM_IDLE) return;
    char *out = applet_run1("basename", clipboard);
    if (!out) return;
    out[strcspn(out, "\n")] = '\0';
    char dest[700];
    snprintf(dest, sizeof dest, "%s/%s", cwd, out);
    free(out);
    int st;
    if (clipmode == FM_COPY)
        st = applet_status("cp", (char *const[]){(char *)"-r",
                                                  (char *)clipboard, (char *)dest, NULL});
    else
        st = applet_status("mv", (char *const[]){(char *)clipboard, (char *)dest, NULL});
    if (st == 0) {
        snprintf(status, sizeof status, clipmode == FM_COPY ? "pegado (copia)" : "pegado (movido)");
        if (clipmode == FM_CUT) clipmode = FM_IDLE;
    } else snprintf(status, sizeof status, "pegado falló (%d)", st);
    reload();
}

static void op_rename(void) {
    char *p = sel_path();
    if (!p) return;
    /* renombrado rápido: <nombre>.ren via mv */
    char dest[700];
    snprintf(dest, sizeof dest, "%s.ren", p);
    int st = applet_status("mv", (char *const[]){(char *)p, (char *)dest, NULL});
    snprintf(status, sizeof status, st == 0 ? "renombrado (mira *.ren)" : "mv falló (%d)", st);
    reload();
}

static void op_delete(void) {
    char *p = sel_path();
    if (!p) return;
    char *name = basename(p);
    if (strcmp(name, "..") == 0 || strcmp(name, ".") == 0) return;
    int st = applet_status("rm", (char *const[]){(char *)"-r", (char *)p, NULL});
    if (st == 0) snprintf(status, sizeof status, "borrado: %s", name);
    else snprintf(status, sizeof status, "rm falló (%d)", st);
    reload();
}

static void op_compress(void) {
    char *p = sel_path();
    if (!p) return;
    char *name = basename(p);
    char tgz[600];
    snprintf(tgz, sizeof tgz, "%s.tar.gz", name);
    int st = applet_status("tar", (char *const[]){(char *)"-czf", (char *)tgz,
                                                   (char *)"-C", (char *)cwd,
                                                   (char *)name, NULL});
    if (st == 0) snprintf(status, sizeof status, "comprimido: %s", tgz);
    else snprintf(status, sizeof status, "tar falló (%d)", st);
    reload();
}

static void op_extract(void) {
    char *p = sel_path();
    if (!p) return;
    const char *name = entries[sel];
    int st = -1;
    size_t nl = strlen(name);
    if (nl > 7 && strcmp(name + nl - 7, ".tar.gz") == 0)
        st = applet_status("tar", (char *const[]){(char *)"-xzf", (char *)p,
                                                   (char *)"-C", (char *)cwd, NULL});
    else if (nl > 4 && strcmp(name + nl - 4, ".tgz") == 0)
        st = applet_status("tar", (char *const[]){(char *)"-xzf", (char *)p,
                                                   (char *)"-C", (char *)cwd, NULL});
    else if (nl > 4 && strcmp(name + nl - 4, ".zip") == 0)
        st = applet_status("unzip", (char *const[]){(char *)"-o", (char *)p,
                                                    (char *)"-d", (char *)cwd, NULL});
    if (st < 0) snprintf(status, sizeof status, "formato no soportado");
    else snprintf(status, sizeof status, st == 0 ? "descomprimido" : "extracción falló (%d)", st);
    reload();
}

static void op_open(void) {
    char *p = sel_path();
    if (!p) return;
    bool d = is_dir(p);
    if (d) {
        char *name = basename(p);
        if (strcmp(name, "..") == 0) {
            char *parent = dirname(cwd);
            snprintf(cwd, sizeof cwd, "%s", parent);
        } else {
            size_t cl = strlen(cwd);
            snprintf(cwd + cl, sizeof cwd - cl, "%s", p + cl);
        }
        reload();
        snprintf(status, sizeof status, "%s", cwd);
    } else {
        /* xdg-open con argv seguro; fallback: ver en el terminal */
        pid_t pid = fork();
        if (pid == 0) {
            execlp("xdg-open", "xdg-open", p, (char *)NULL);
            _exit(127);
        }
        int st = 0;
        waitpid(pid, &st, 0);
        if (WIFEXITED(st) && WEXITSTATUS(st) != 0)
            snprintf(status, sizeof status, "sin xdg-open para abrir el archivo");
        else
            snprintf(status, sizeof status, "abierto: %s", basename(p));
    }
}

/* ---------- UI ---------- */
#define BTN_N 9
static const char *btn_labels[BTN_N] = {
    "Abrir", "Copiar", "Cortar", "Pegar", "Renombrar",
    "Nueva carp.", "Comprimir", "Extraer", "Borrar"
};

static void fm_draw(UiCtx *u) {
    ui_clear(u);
    /* toolbar */
    int bx = 4;
    for (int i = 0; i < BTN_N; i++) {
        ui_button(u, bx, 4, 82, 24, btn_labels[i], false);
        bx += 86;
    }
    /* cwd bar */
    ui_bevel(u, 4, 32, u->w - 8, 22, true);
    ui_text(u, 8, 47, cwd);
    /* file list */
    int ly = 60;
    int rows = (u->h - ly - 24) / ROW_H;
    for (int i = 0; i < rows; i++) {
        int idx = top + i;
        if (idx >= nentries) break;
        int rowy = ly + i * ROW_H;
        if (idx == sel) {
            XSetForeground(u->dpy, u->gc, u->c_accent);
            XFillRectangle(u->dpy, u->win, u->gc, 6, rowy, (unsigned)u->w - 12, ROW_H);
        }
        XSetForeground(u->dpy, u->gc, idx == sel ? u->c_white : u->c_fg);
        XDrawString(u->dpy, u->win, u->gc, 10, rowy + 13,
                    entries[idx], (int)strlen(entries[idx]));
    }
    /* status bar */
    ui_bevel(u, 4, u->h - 22, u->w - 8, 18, true);
    ui_text(u, 8, u->h - 9, status);
    ui_flush(u);
}

int main(void) {
    UiCtx u;
    ui_init(&u, 800, 480, "Explorador de archivos — W3M");
    if (getcwd(cwd, sizeof cwd) == NULL) snprintf(cwd, sizeof cwd, "/");
    reload();

    XEvent ev;
    for (;;) {
        if (!ui_next_event(&u, &ev)) break;
        if (ev.type == Expose) { fm_draw(&u); continue; }
        if (ev.type == ButtonPress && ev.xbutton.y >= 4 && ev.xbutton.y <= 28) {
            int b = (ev.xbutton.x - 4) / 86;
            if (ev.xbutton.x >= 4 && b < BTN_N
                && ev.xbutton.x - 4 - b * 86 <= 82) {
                switch (b) {
                case 0: op_open(); break;
                case 1: op_copy(); break;
                case 2: op_cut(); break;
                case 3: op_paste(); break;
                case 4: op_rename(); break;
                case 5: op_crear_carpeta(); break;
                case 6: op_compress(); break;
                case 7: op_extract(); break;
                case 8: op_delete(); break;
                }
                fm_draw(&u);
            }
        } else if (ev.type == ButtonPress && ev.xbutton.y >= 60) {
            int row = (ev.xbutton.y - 60) / ROW_H;
            int idx = top + row;
            if (idx < nentries) {
                sel = idx;
                fm_draw(&u);
                if (ev.xbutton.button == Button2)  /* doble-clic aprox.: botón central */
                    op_open(), fm_draw(&u);
            }
        } else if (ev.type == KeyPress) {
            char kb[8]; KeySym ks;
            XLookupString(&ev.xkey, kb, sizeof kb, &ks, NULL);
            if (ks == XK_Down && sel < nentries - 1) { sel++; fm_draw(&u); }
            else if (ks == XK_Up && sel > 0) { sel--; fm_draw(&u); }
            else if (ks == XK_Return) { op_open(); fm_draw(&u); }
        }
    }
    ui_close(&u);
    return 0;
}
