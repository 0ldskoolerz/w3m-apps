/* w3m-task — administrador de tareas estilo Win 95: lista procesos
   (busybox ps), memoria (busybox free), matar con kill. */

#include "ui.h"
#include "applet.h"

#define PROC_MAX 256
#define NAME_LEN 64
#define ROW_H 18

static char procs[PROC_MAX][128];   /* "PID NAME" per line */
static int nprocs = 0;
static int sel = 0;
static char meminfo[128] = "";

static void reload(void) {
    nprocs = 0;
    char *out = applet_run("ps", (char *const[]){(char *)"-o", (char *)"pid,user,comm", NULL});
    if (!out) return;
    char *save = NULL;
    int first = 1;
    for (char *tok = strtok_r(out, "\n", &save);
         tok && nprocs < PROC_MAX; tok = strtok_r(NULL, "\n", &save)) {
        if (first) { first = 0; continue; }   /* header */
        snprintf(procs[nprocs], 128, "%s", tok);
        nprocs++;
    }
    free(out);
    char *mem = applet_run1("free", "-h");
    if (mem) {
        char *first_line = strtok(mem, "\n");
        char *second_line = first_line ? strtok(NULL, "\n") : NULL;
        if (second_line) snprintf(meminfo, sizeof meminfo, "Mem: %s", second_line);
        free(mem);
    }
}

static void kill_selected(void) {
    if (sel < 0 || sel >= nprocs) return;
    int pid = atoi(procs[sel]);
    if (pid <= 1) return;
    char spid[16];
    snprintf(spid, sizeof spid, "%d", pid);
    applet_status("kill", (char *const[]){(char *)"-9", (char *)spid, NULL});
    reload();
}

static void task_draw(UiCtx *u) {
    ui_clear(u);
    ui_button(u, 4, 4, 100, 24, "Finalizar tarea", false);
    ui_button(u, 110, 4, 100, 24, "Actualizar", false);
    ui_text(u, 220, 20, meminfo);
    /* list */
    int ly = 36;
    int rows = (u->h - ly - 4) / ROW_H;
    for (int i = 0; i < rows; i++) {
        int idx = i;
        if (idx >= nprocs) break;
        int rowy = ly + i * ROW_H;
        if (idx == sel) {
            XSetForeground(u->dpy, u->gc, u->c_accent);
            XFillRectangle(u->dpy, u->win, u->gc, 6, rowy, (unsigned)u->w - 12, ROW_H);
        }
        XSetForeground(u->dpy, u->gc, idx == sel ? u->c_white : u->c_fg);
        XDrawString(u->dpy, u->win, u->gc, 10, rowy + 13,
                    procs[idx], (int)strlen(procs[idx]));
    }
    ui_flush(u);
}

int main(void) {
    UiCtx u;
    ui_init(&u, 600, 420, "Administrador de tareas — W3M");
    reload();
    task_draw(&u);

    XEvent ev;
    for (;;) {
        if (!ui_next_event(&u, &ev)) break;
        if (ev.type == Expose) { task_draw(&u); continue; }
        if (ev.type == ButtonPress) {
            if (ev.xbutton.y >= 4 && ev.xbutton.y <= 28) {
                if (ev.xbutton.x < 104) kill_selected();
                else if (ev.xbutton.x >= 110 && ev.xbutton.x <= 210) reload();
                task_draw(&u);
            } else if (ev.xbutton.y >= 36) {
                int row = (ev.xbutton.y - 36) / ROW_H;
                if (row < nprocs) { sel = row; task_draw(&u); }
            }
        }
    }
    ui_close(&u);
    return 0;
}
