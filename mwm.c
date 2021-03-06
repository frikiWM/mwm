#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
//#include <X11/Xproto.h>
//#include <X11/Xutil.h>
//#include <errno.h>
//#include <locale.h>
//#include <signal.h>
//#include <string.h>
//#include <unistd.h>
//#ifdef XINERAMA
//#include <X11/extensions/Xinerama.h>
//#endif
*/

/* Functions */
static void handler_xevent(XEvent *e);
static void handler_configurerequest(XEvent *e);

/* Variables */
static Display *dpy;
static int screen;
static int sw;
static int sh;
static Window root;
enum { WMProtocols, WMDelete, WMState, WMLast };
enum { NetSupported, NetWMName, NetWMState, NetWMFullscreen, NetLast };
enum { CurNormal, CurResize, CurMove, CurLast };
static Atom wmatom[WMLast], netatom[NetLast];
static Cursor cursor[CurLast];
static void (*handler[LASTEvent]) (XEvent *);

void init() {
/*	handler[ButtonPress] = handler_xevent;*/ /*4*/
	handler[ClientMessage] = handler_xevent; /*33*/
	handler[ConfigureRequest] = handler_configurerequest; /*23*/
	handler[ConfigureNotify] = handler_xevent; /*22*/
	handler[DestroyNotify] = handler_xevent; /*17*/
	handler[EnterNotify] = handler_xevent; /*7*/
	handler[Expose] = handler_xevent; /*12*/
	handler[FocusIn] = handler_xevent; /*9*/
	handler[KeyPress] = handler_xevent; /*2*/
	handler[MappingNotify] = handler_xevent; /*34*/
	handler[MapRequest] = handler_xevent; /*20*/
	handler[PropertyNotify] = handler_xevent; /*28*/
	handler[UnmapNotify] = handler_xevent; /*18*/
}

void manage(Window w, XWindowAttributes *wa) {
	XWindowChanges wc;

	wc.border_width = 1;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, w, 0xFF0000);
	XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
	XMoveResizeWindow(dpy, w, 0, 0, sw-wc.border_width*2, sh-wc.border_width*2);/* xywh - 'full-screen' */ 
	XMapWindow(dpy, w);
}

void scan(void) {
	unsigned int i, num;
	Window d1, d2, *wins = NULL;
	XWindowAttributes wa;

	if(XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for(i = 0; i < num; i++) {
			if(!XGetWindowAttributes(dpy, wins[i], &wa)
			|| wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if(wa.map_state == IsViewable)
				manage(wins[i], &wa);
		}
		for(i = 0; i < num; i++) { /* now the transients */
			if(!XGetWindowAttributes(dpy, wins[i], &wa))
				continue;
			if(XGetTransientForHint(dpy, wins[i], &d1) && wa.map_state == IsViewable)
				manage(wins[i], &wa);
		}
		if(wins)
			XFree(wins);
	}
}

void handler_configurerequest(XEvent *e) {
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XMoveResizeWindow(dpy, ev->window, 0, 0, 300, 400);
}

void handler_xevent(XEvent *e) {
}

void die(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void sigchld(int unused) {
        if(signal(SIGCHLD, sigchld) == SIG_ERR)
                die("Can't install SIGCHLD handler");
        while(0 < waitpid(-1, NULL, WNOHANG));
}

void setup(void) {
	XSetWindowAttributes wa;

	/* clean up any zombies immediately */
	sigchld(0);

	/* init screen */
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	/* init atoms */
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
	netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
	netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
	netatom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	/* init cursors */
	cursor[CurNormal] = XCreateFontCursor(dpy, XC_left_ptr);
	cursor[CurResize] = XCreateFontCursor(dpy, XC_sizing);
	cursor[CurMove] = XCreateFontCursor(dpy, XC_fleur);
	/* EWMH support per view */
	XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
			PropModeReplace, (unsigned char *) netatom, NetLast);
	/* select for events */
	wa.cursor = cursor[CurNormal];
	wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask|ButtonPressMask
		|EnterWindowMask|LeaveWindowMask|StructureNotifyMask
		|PropertyChangeMask;
	XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
	XSelectInput(dpy, root, wa.event_mask);
}

void run(void) {
	XEvent ev;
	/* main event loop */
	while(!XNextEvent(dpy, &ev)) {
		if(handler[ev.type]) {
			fprintf(stderr, "mwm: run() [handled: %d]\n", ev.type);
			handler[ev.type](&ev); /* call handler */
		} else {
			fprintf(stderr, "mwm: run() [unhandled: %d]\n", ev.type);
		}
	}
}

int main(int argc, char *argv[]) {
	if(!(dpy = XOpenDisplay(NULL)))
		die("mwm: cannot open display\n");
	init();

	setup();
	scan();
	run();

	XCloseDisplay(dpy);
	return 0;
}
