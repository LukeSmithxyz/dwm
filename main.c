/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>

#include "dwm.h"

/********** CUSTOMIZE **********/

char *tags[TLast] = {
	[Tscratch] = "scratch",
	[Tdev] = "dev",
	[Tirc] = "irc",
	[Twww] = "www",
	[Twork] = "work",
};

/********** CUSTOMIZE **********/

/* X structs */
Display *dpy;
Window root, barwin;
Atom wm_atom[WMLast], net_atom[NetLast];
Cursor cursor[CurLast];
Bool running = True;
Bool issel;

int tsel = Tdev; /* default tag */
int screen, sx, sy, sw, sh, bx, by, bw, bh, mw;
char stext[1024];

DC dc = {0};
Client *clients = NULL;
Client *sel = NULL;

static Bool other_wm_running;
static const char version[] =
	"dwm-" VERSION ", (C)opyright MMVI Anselm R. Garbe\n";
static int (*x_error_handler) (Display *, XErrorEvent *);

static void
usage() {	error("usage: dwm [-v]\n"); }

static void
scan_wins()
{
	unsigned int i, num;
	Window *wins;
	XWindowAttributes wa;
	Window d1, d2;

	if(XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for(i = 0; i < num; i++) {
			if(!XGetWindowAttributes(dpy, wins[i], &wa))
				continue;
			if(wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if(wa.map_state == IsViewable)
				manage(wins[i], &wa);
		}
	}
	if(wins)
		XFree(wins);
}

static int
win_property(Window w, Atom a, Atom t, long l, unsigned char **prop)
{
	Atom real;
	int format;
	unsigned long res, extra;
	int status;

	status = XGetWindowProperty(dpy, w, a, 0L, l, False, t, &real, &format,
			&res, &extra, prop);

	if(status != Success || *prop == 0) {
		return 0;
	}
	if(res == 0) {
		free((void *) *prop);
	}
	return res;
}

int
win_proto(Window w)
{
	unsigned char *protocols;
	long res;
	int protos = 0;
	int i;

	res = win_property(w, wm_atom[WMProtocols], XA_ATOM, 20L, &protocols);
	if(res <= 0) {
		return protos;
	}
	for(i = 0; i < res; i++) {
		if(protocols[i] == wm_atom[WMDelete])
			protos |= WM_PROTOCOL_DELWIN;
	}
	free((char *) protocols);
	return protos;
}

void
send_message(Window w, Atom a, long value)
{
	XEvent e;

	e.type = ClientMessage;
	e.xclient.window = w;
	e.xclient.message_type = a;
	e.xclient.format = 32;
	e.xclient.data.l[0] = value;
	e.xclient.data.l[1] = CurrentTime;
	XSendEvent(dpy, w, False, NoEventMask, &e);
	XFlush(dpy);
}

/*
 * There's no way to check accesses to destroyed windows, thus
 * those cases are ignored (especially on UnmapNotify's).
 * Other types of errors call Xlib's default error handler, which
 * calls exit().
 */
int
error_handler(Display *dpy, XErrorEvent *error)
{
	if(error->error_code == BadWindow
			|| (error->request_code == X_SetInputFocus
				&& error->error_code == BadMatch)
			|| (error->request_code == X_PolyText8
				&& error->error_code == BadDrawable)
			|| (error->request_code == X_PolyFillRectangle
				&& error->error_code == BadDrawable)
			|| (error->request_code == X_PolySegment
				&& error->error_code == BadDrawable)
			|| (error->request_code == X_ConfigureWindow
				&& error->error_code == BadMatch)
			|| (error->request_code == X_GrabKey
				&& error->error_code == BadAccess))
		return 0;
	fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
			error->request_code, error->error_code);
	return x_error_handler(dpy, error); /* may call exit() */
}

/*
 * Startup Error handler to check if another window manager
 * is already running.
 */
static int
startup_error_handler(Display *dpy, XErrorEvent *error)
{
	other_wm_running = True;
	return -1;
}

static void
cleanup()
{
	while(sel) {
		resize(sel, True);
		unmanage(sel);
	}
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
}

void
quit(Arg *arg)
{
	running = False;
}

int
main(int argc, char *argv[])
{
	int i;
	XSetWindowAttributes wa;
	unsigned int mask;
	Window w;
	XEvent ev;

	/* command line args */
	for(i = 1; (i < argc) && (argv[i][0] == '-'); i++) {
		switch (argv[i][1]) {
		case 'v':
			fprintf(stdout, "%s", version);
			exit(0);
			break;
		default:
			usage();
			break;
		}
	}

	dpy = XOpenDisplay(0);
	if(!dpy)
		error("dwm: cannot connect X server\n");

	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);

	/* check if another WM is already running */
	other_wm_running = False;
	XSetErrorHandler(startup_error_handler);
	/* this causes an error if some other WM is running */
	XSelectInput(dpy, root, SubstructureRedirectMask);
	XFlush(dpy);

	if(other_wm_running)
		error("dwm: another window manager is already running\n");

	XSetErrorHandler(0);
	x_error_handler = XSetErrorHandler(error_handler);

	/* init atoms */
	wm_atom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wm_atom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	net_atom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	net_atom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
	XChangeProperty(dpy, root, net_atom[NetSupported], XA_ATOM, 32,
			PropModeReplace, (unsigned char *) net_atom, NetLast);

	/* init cursors */
	cursor[CurNormal] = XCreateFontCursor(dpy, XC_left_ptr);
	cursor[CurResize] = XCreateFontCursor(dpy, XC_sizing);
	cursor[CurMove] = XCreateFontCursor(dpy, XC_fleur);

	update_keys();

	/* style */
	dc.bg = initcolor(BGCOLOR);
	dc.fg = initcolor(FGCOLOR);
	dc.border = initcolor(BORDERCOLOR);
	initfont(FONT);

	sx = sy = 0;
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	mw = (sw * MASTERW) / 100;

	wa.override_redirect = 1;
	wa.background_pixmap = ParentRelative;
	wa.event_mask = ButtonPressMask | ExposureMask;

	bx = by = 0;
	bw = sw;
	dc.h = bh = dc.font.height + 4;
	barwin = XCreateWindow(dpy, root, bx, by, bw, bh, 0, DefaultDepth(dpy, screen),
			CopyFromParent, DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
	XDefineCursor(dpy, barwin, cursor[CurNormal]);
	XMapRaised(dpy, barwin);

	issel = XQueryPointer(dpy, root, &w, &w, &i, &i, &i, &i, &mask);

	wa.event_mask = SubstructureRedirectMask | EnterWindowMask \
					| LeaveWindowMask;
	wa.cursor = cursor[CurNormal];

	XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &wa);

	dc.drawable = XCreatePixmap(dpy, root, sw, bh, DefaultDepth(dpy, screen));
	dc.gc = XCreateGC(dpy, root, 0, 0);

	strcpy(stext, "dwm-"VERSION);
	scan_wins();
	draw_bar();

	while(running) {
		XNextEvent(dpy, &ev);
		if(handler[ev.type])
			(handler[ev.type])(&ev); /* call handler */
	}

	cleanup();
	XCloseDisplay(dpy);

	return 0;
}
