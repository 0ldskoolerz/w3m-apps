#ifndef X11_STUB_H
typedef struct _XDisplay Display;
#define X11_STUB_H
/* Type-check stub mirroring the subset of Xlib used by W3M.
   Only used for verification where X11 headers are unavailable. */
#include <stdbool.h>

typedef unsigned long Window;
typedef unsigned long Drawable;
typedef unsigned long Pixmap;
typedef unsigned long Atom;
typedef unsigned long Colormap;
typedef unsigned long Time;
typedef unsigned long KeySym;
typedef unsigned long XID;
typedef XID Cursor;
typedef int Bool;
typedef char *XPointer;

#define False 0
#define True 1
#define None 0
#define NoEventMask 0L
#define CurrentTime 0L

#define CWX 0x0001
#define CWY 0x0002
#define CWWidth 0x0004
#define CWHeight 0x0008
#define CWBorderWidth 0x0010

#define ButtonReleaseMask (1L<<10)
#define PointerMotionMask (1L<<11)
#define ExposureMask (1L<<15)
#define KeyPressMask (1L<<0)
#define ButtonPressMask (1L<<2)
#define SubstructureNotifyMask (1L<<19)
#define StructureNotifyMask (1L<<17)
#define SubstructureRedirectMask (1L<<20)
#define Mod1Mask (1<<3)

#define GrabModeAsync 0
#define RevertToPointerRoot 1

#define XK_Tab 0xff09

#define ClientMessage 33
#define MapRequest 20
#define ConfigureRequest 21
#define DestroyNotify 17
#define UnmapNotify 18
#define Expose 12
#define Button1 1
#define Button2 2
#define Button3 3
#define ButtonPress 4
#define ButtonRelease 5
#define KeyPress 2

typedef struct {
    int type; Window window; Window message_type;
    int format; union { char b[20]; short s[10]; long l[5]; } data;
} XClientMessageEvent;

typedef struct { int x, y; } XPoint;
typedef struct { short x, y; unsigned short width, height; } XRectangle;
typedef struct { int type; unsigned long serial; Window window; } XAnyEvent;
typedef struct { int type; Window window; int x, y, width, height, count; } XExposeEvent;
typedef struct { int type; Window window; int x, y; unsigned int button, state; } XButtonEvent;
typedef struct { int type; Window window; int x, y; unsigned int state; } XKeyEvent;
typedef struct { int type; Window window; int x, y; } XMapRequestEvent;
typedef struct { int type; Window window; Window parent; int x, y; } XReparentEvent;
typedef struct { int type; Window window; } XDestroyWindowEvent;
typedef struct { int type; Window window; } XUnmapEvent;
typedef struct {
    int type; Window window;
    int x, y, width, height, border_width;
    Window above; unsigned long value_mask;
} XConfigureRequestEvent;
typedef struct {
    int type; Window window; Window root, subwindow;
    int x_root, y_root, x, y; unsigned int state;
} XMotionEvent;

typedef union _XEvent {
    int type;
    XAnyEvent xany;
    XExposeEvent xexpose;
    XButtonEvent xbutton;
    XKeyEvent xkey;
    XMapRequestEvent xmaprequest;
    XReparentEvent xreparent;
    XDestroyWindowEvent xdestroywindow;
    XUnmapEvent xunmap;
    XConfigureRequestEvent xconfigurerequest;
    XMotionEvent xmotion;
    XClientMessageEvent xclient;
    long pad[24];
} XEvent;

typedef struct { unsigned char *value; } XTextProperty;
typedef struct { int x, y, width, height, border_width, depth, screen; Window root; Bool override_redirect; } XWindowAttributes;
typedef struct { int x, y, width, height, border_width; Window sibling; int stack_mode; unsigned long value_mask; } XWindowChanges;
typedef struct { unsigned long pixel; unsigned short red, green, blue; int flags; } XColor;
typedef struct _XFontStruct { unsigned long fid; int ascent, descent; int direction; } XFontStruct;
typedef struct _XGC { XID gid; } *GC;
typedef struct { char *chars; } XChar2b;

Display *XOpenDisplay(const char *);
int XCloseDisplay(Display *);
int XSync(Display *, Bool);
int XFlush(Display *);
int XPending(Display *);
int XNextEvent(Display *, XEvent *);
int XMaskEvent(Display *, long, XEvent *);
int XSelectInput(Display *, Window, long);
int XGrabPointer(Display *, Window, Bool, unsigned int, int, int, Window, Cursor, Time);
int XUngrabPointer(Display *, Time);
int XGrabKey(Display *, int, unsigned int, Window, Bool, int, int);
Window XCreateSimpleWindow(Display *, Window, int, int, unsigned int, unsigned int, unsigned int, unsigned long, unsigned long);
int XDestroyWindow(Display *, Window);
int XMapWindow(Display *, Window);
int XUnmapWindow(Display *, Window);
int XMoveWindow(Display *, Window, int, int);
int XResizeWindow(Display *, Window, unsigned int, unsigned int);
int XMoveResizeWindow(Display *, Window, int, int, unsigned int, unsigned int);
int XConfigureWindow(Display *, Window, unsigned int, XWindowChanges *);
int XReparentWindow(Display *, Window, Window, int, int);
int XAddToSaveSet(Display *, Window);
int XRemoveFromSaveSet(Display *, Window);
int XRaiseWindow(Display *, Window);
int XSetInputFocus(Display *, Window, int, Time);
int XClearArea(Display *, Window, int, int, unsigned int, unsigned int, Bool);
int XClearWindow(Display *, Window);
int XQueryPointer(Display *, Window, Window *, Window *, int *, int *, int *, int *, unsigned int *);
int XGetWindowAttributes(Display *, Window, XWindowAttributes *);
Bool XGetWMName(Display *, Window, XTextProperty *);
int XStoreName(Display *, Window, const char *);
Atom XInternAtom(Display *, const char *, Bool);
Atom *XListProtocols(Display *, Window, int *);
int XSendEvent(Display *, Window, Bool, long, XEvent *);
Bool XAllocNamedColor(Display *, Colormap, const char *, XColor *, XColor *);
Bool XParseColor(Display *, Colormap, const char *, XColor *);
Bool XAllocColor(Display *, Colormap, XColor *);
GC XCreateGC(Display *, Drawable, unsigned long, void *);
int XSetFont(Display *, GC, XID);
XFontStruct *XLoadQueryFont(Display *, const char *);
int XSetForeground(Display *, GC, unsigned long);
int XFillRectangle(Display *, Drawable, GC, int, int, unsigned int, unsigned int);
int XDrawString(Display *, Drawable, GC, int, int, const char *, int);
int XDrawLine(Display *, Drawable, GC, int, int, int, int);
int XTextWidth(XFontStruct *, const char *, int);
KeySym XLookupKeysym(XKeyEvent *, int);
int XKeysymToKeycode(Display *, KeySym);
int XSetErrorHandler(void *);
int XSetIOErrorHandler(void *);
void XFree(void *);


#define XA_PRIMARY 1
#define XA_WINDOW 33
#define XA_ATOM 4
#define XA_CARDINAL 6
#define PropModeReplace 0
#define PropModeAppend 2

int XChangeProperty(Display *, Window, Atom, Atom, int, int, const unsigned char *, int);
int XDeleteProperty(Display *, Window, Atom);
int XMapRaised(Display *, Window);


Atom *XListProtocols(Display *, Window, int *);
int XSetWMProtocols(Display *, Window, Atom *, int);
int XFreeGC(Display *, GC);


#define XK_Return 0xff0d
#define XK_BackSpace 0xff08
#define XK_Left 0xff51
#define XK_Right 0xff53
#define XK_Up 0xff52
#define XK_Down 0xff54
int XLookupString(XKeyEvent *, char *, int, KeySym *, void *);

int DefaultScreen(Display *);
Window RootWindow(Display *, int);
int DisplayWidth(Display *, int);
int DisplayHeight(Display *, int);
Colormap DefaultColormap(Display *, int);
unsigned long BlackPixel(Display *, int);
unsigned long WhitePixel(Display *, int);

#endif
