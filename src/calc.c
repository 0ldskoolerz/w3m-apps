/* w3m-calc — calculadora Win 3.x: botones + display. La evaluación la
   hace `busybox awk 'BEGIN{print expr}'` con whitelist de caracteres. */

#include "ui.h"
#include "applet.h"

#define EXPR_MAX 128
static char expr[EXPR_MAX] = "";
static char result[64] = "0";

static bool valid_char(char c) {
    return (c >= '0' && c <= '9') || c == '+' || c == '-' || c == '*' ||
           c == '/' || c == '%' || c == '.' || c == '(' || c == ' ';
}

/* evaluate properly: applet_run con argv controlado */
static void evaluate2(void) {
    if (!expr[0]) return;
    for (const char *p = expr; *p; p++)
        if (!valid_char(*p)) { snprintf(result, sizeof result, "error"); return; }
    char prog[192];
    snprintf(prog, sizeof prog, "BEGIN{print (%s)}", expr);
    char *args[] = {(char *)prog, NULL};
    char *out = applet_run("awk", args);
    if (!out) { snprintf(result, sizeof result, "error"); return; }
    out[strcspn(out, "\n")] = '\0';
    snprintf(result, sizeof result, "%s", out);
    free(out);
}

static const char *keys[] = {
    "7", "8", "9", "/",
    "4", "5", "6", "*",
    "1", "2", "3", "-",
    "0", ".", "=", "+",
};
#define KEYS_N 16
#define KEY_W 52
#define KEY_H 38

static void calc_draw(UiCtx *u) {
    ui_clear(u);
    /* display */
    ui_bevel(u, 8, 8, u->w - 16, 34, false);
    XSetForeground(u->dpy, u->gc, u->c_white);
    XFillRectangle(u->dpy, u->win, u->gc, 10, 10, (unsigned)u->w - 20, 30);
    char disp[EXPR_MAX + 64];
    snprintf(disp, sizeof disp, "%s = %s", expr, result);
    XSetForeground(u->dpy, u->gc, u->c_fg);
    int dw = XTextWidth(u->font, disp, (int)strlen(disp));
    XDrawString(u->dpy, u->win, u->gc, u->w - 16 - dw, 30, disp, (int)strlen(disp));
    /* keypad 4x4 */
    for (int i = 0; i < KEYS_N; i++) {
        int col = i % 4, row = i / 4;
        ui_button(u, 8 + col * (KEY_W + 6), 50 + row * (KEY_H + 6),
                  KEY_W, KEY_H, keys[i], false);
    }
    ui_button(u, 8, 50 + 4 * (KEY_H + 6), KEY_W * 2 + 6, KEY_H, "C", false);
    ui_flush(u);
}

int main(void) {
    UiCtx u;
    ui_init(&u, 260, 280, "Calculadora — W3M");
    calc_draw(&u);

    XEvent ev;
    for (;;) {
        if (!ui_next_event(&u, &ev)) break;
        if (ev.type == Expose) { calc_draw(&u); continue; }
        if (ev.type == ButtonPress) {
            int kx = (ev.xbutton.x - 8) / (KEY_W + 6);
            int ky = (ev.xbutton.y - 50) / (KEY_H + 6);
            const char *pressed = NULL;
            if (ky >= 0 && ky < 4 && kx >= 0 && kx < 4)
                pressed = keys[ky * 4 + kx];
            else if (ky == 4 && kx < 2)
                pressed = "C";
            if (pressed) {
                if (pressed[0] == 'C') { expr[0] = '\0'; snprintf(result, sizeof result, "0"); }
                else if (pressed[0] == '=') evaluate2();
                else {
                    size_t el = strlen(expr);
                    if (el < EXPR_MAX - 2) { expr[el] = pressed[0]; expr[el + 1] = '\0'; }
                }
                calc_draw(&u);
            }
        }
    }
    ui_close(&u);
    return 0;
}
