/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>

#include "wm.h"

/* X structs */
Display *dpy;
Window root, barwin;
Atom wm_atom[WMLast], net_atom[NetLast];
Cursor cursor[CurLast];
XRectangle rect, barrect;
Bool running = True;
Client *client = NULL;

char *bartext, tag[256];
int screen, sel_screen;
unsigned int lock_mask, numlock_mask;

/* draw structs */
Brush brush = {0};

enum { WM_PROTOCOL_DELWIN = 1 };

static Bool other_wm_running;
static int (*x_error_handler) (Display *, XErrorEvent *);
static char version[] = "gridwm - " VERSION ", (C)opyright MMVI Anselm R. Garbe\n";

static void
usage()
{
	fputs("usage: gridwm [-v]\n", stderr);
	exit(1);
}

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
				manage(create_client(wins[i], &wa));
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

	if(status != Success || *prop == NULL) {
		return 0;
	}
	if(res == 0)
		free((void *) *prop);
	return res;
}

int
win_proto(Window w)
{
	Atom *protocols;
	long res;
	int protos = 0;
	int i;

	res = win_property(w, wm_atom[WMProtocols], XA_ATOM, 20L,
			((unsigned char **) &protocols));
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

/*
 * There's no way to check accesses to destroyed windows, thus
 * those cases are ignored (especially on UnmapNotify's).
 * Other types of errors call Xlib's default error handler, which
 * calls exit().
 */
static int
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
	fprintf(stderr, "gridwm: fatal error: request code=%d, error code=%d\n",
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
init_lock_keys()
{
	XModifierKeymap *modmap;
	KeyCode numlock;
	int i;
	static int masks[] = {
		ShiftMask, LockMask, ControlMask, Mod1Mask,
		Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
	};

	numlock_mask = 0;
	modmap = XGetModifierMapping(dpy);
	numlock = XKeysymToKeycode(dpy, XStringToKeysym("Num_Lock"));

	if(modmap && modmap->max_keypermod > 0) {
		int max = (sizeof(masks) / sizeof(int)) * modmap->max_keypermod;
		for(i = 0; i < max; i++)
			if(numlock && (modmap->modifiermap[i] == numlock))
				numlock_mask = masks[i / modmap->max_keypermod];
	}
	XFreeModifiermap(modmap);

	lock_mask = 255 & ~(numlock_mask | LockMask);
}

static void
cleanup()
{
	/*
	Client *c;
	for(c=client; c; c=c->next)
		reparent_client(c, root, c->sel->rect.x, c->sel->rect.y);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	*/
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
		error("gridwm: cannot connect X server\n");

	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);

	/* check if another WM is already running */
	other_wm_running = False;
	XSetErrorHandler(startup_error_handler);
	/* this causes an error if some other WM is running */
	XSelectInput(dpy, root, SubstructureRedirectMask);
	XFlush(dpy);

	if(other_wm_running)
		error("gridwm: another window manager is already running\n");

	rect.x = rect.y = 0;
	rect.width = DisplayWidth(dpy, screen);
	rect.height = DisplayHeight(dpy, screen);
	sel_screen = XQueryPointer(dpy, root, &w, &w, &i, &i, &i, &i, &mask);

	XSetErrorHandler(0);
	x_error_handler = XSetErrorHandler(error_handler);

	/* init atoms */
	wm_atom[WMState] = XInternAtom(dpy, "WM_STATE", False);
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

	init_lock_keys();

	brush.drawable = XCreatePixmap(dpy, root, rect.width, rect.height,
			DefaultDepth(dpy, screen));
	brush.gc = XCreateGC(dpy, root, 0, 0);

	/* style */
	loadcolors(dpy, screen, &brush, BGCOLOR, FGCOLOR, BORDERCOLOR);
	loadfont(dpy, &brush.font, FONT);

	wa.override_redirect = 1;
	wa.background_pixmap = ParentRelative;
	wa.event_mask = ExposureMask;

	barrect = rect;
	barrect.height = labelheight(&brush.font);
	barrect.y = rect.height - barrect.height;
	barwin = XCreateWindow(dpy, root, barrect.x, barrect.y,
			barrect.width, barrect.height, 0, DefaultDepth(dpy, screen),
			CopyFromParent, DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
	bartext = NULL;
	XDefineCursor(dpy, barwin, cursor[CurNormal]);
	XMapRaised(dpy, barwin);
	draw_bar();

	wa.event_mask = SubstructureRedirectMask | EnterWindowMask | LeaveWindowMask;
	wa.cursor = cursor[CurNormal];
	XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &wa);

	scan_wins();

	while(running) {
		XNextEvent(dpy, &ev);
		if(handler[ev.type])
			(handler[ev.type]) (&ev); /* call handler */
	}

	cleanup();
	XCloseDisplay(dpy);

	return 0;
}
