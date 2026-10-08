#include "applet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static char *capture_from_pipe(int fd) {
    char *buf = malloc(APPLET_MAX_OUT);
    if (!buf) return NULL;
    size_t n = 0;
    ssize_t r;
    while (n < APPLET_MAX_OUT - 1 &&
           (r = read(fd, buf + n, APPLET_MAX_OUT - 1 - n)) > 0)
        n += (size_t)r;
    buf[n] = '\0';
    return buf;
}

/* run applet; if capture is non-NULL, capture output there (malloc'd);
   returns exit status, or -1 on spawn failure */
static int applet_exec(const char *name, char *const args[],
                       char **capture) {
    int pipefd[2];
    if (pipe(pipefd) != 0) return -1;

    pid_t pid = fork();
    if (pid < 0) { close(pipefd[0]); close(pipefd[1]); return -1; }

    if (pid == 0) {
        /* child: redirect stdout+stderr to pipe, then exec busybox */
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        /* build argv: busybox name a1 a2 ... NULL */
        int argc = 2;
        for (char *const *p = args; p && *p; p++) argc++;
        char **argv = malloc(sizeof(char *) * (size_t)(argc + 1));
        if (!argv) _exit(127);
        argv[0] = (char *)"busybox";
        argv[1] = (char *)name;
        int i = 2;
        for (char *const *p = args; p && *p; p++) argv[i++] = *p;
        argv[i] = NULL;
        execvp(argv[0], argv);
        /* busybox missing: try standalone applet name */
        argv[0] = (char *)name;
        execvp(argv[0], argv);
        _exit(127);
    }

    close(pipefd[1]);
    if (capture) {
        *capture = capture_from_pipe(pipefd[0]);
        close(pipefd[0]);
    } else {
        close(pipefd[0]);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return -1;
}

char *applet_run(const char *name, char *const args[]) {
    char *out = NULL;
    if (applet_exec(name, args, &out) < 0) {
        free(out);
        return NULL;
    }
    if (out && !out[0]) { free(out); return NULL; }
    return out;
}

int applet_status(const char *name, char *const args[]) {
    return applet_exec(name, args, NULL);
}

char *applet_run1(const char *name, const char *a1) {
    char *args[] = {(char *)a1, NULL};
    return applet_run(name, args);
}
char *applet_run2(const char *name, const char *a1, const char *a2) {
    char *args[] = {(char *)a1, (char *)a2, NULL};
    return applet_run(name, args);
}
char *applet_run3(const char *name, const char *a1, const char *a2, const char *a3) {
    char *args[] = {(char *)a1, (char *)a2, (char *)a3, NULL};
    return applet_run(name, args);
}
char *applet_run4(const char *name, const char *a1, const char *a2,
                  const char *a3, const char *a4) {
    char *args[] = {(char *)a1, (char *)a2, (char *)a3, (char *)a4, NULL};
    return applet_run(name, args);
}
