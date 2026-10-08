#ifndef APPLET_H
#define APPLET_H

/* Thin wrapper over busybox applets: run an applet with an argv array
   (no shell, no injection) and capture its stdout+stderr into a buffer.
   Falls back to the standalone applet binary if busybox is missing. */

#include <stdbool.h>

#define APPLET_MAX_OUT (64 * 1024)

/* Run `busybox <name> <args...>` (NULL-terminated argv after name).
   Returns malloc'd output (caller frees), or NULL on spawn failure. */
char *applet_run(const char *name, char *const args[]);

/* Convenience wrappers */
char *applet_run1(const char *name, const char *a1);
char *applet_run2(const char *name, const char *a1, const char *a2);
char *applet_run3(const char *name, const char *a1, const char *a2, const char *a3);
char *applet_run4(const char *name, const char *a1, const char *a2,
                  const char *a3, const char *a4);

/* returns exit status (< 0 on spawn failure) */
int applet_status(const char *name, char *const args[]);

#endif
