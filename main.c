/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "dwm.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>

/* static */

static int (*xerrorxlib)(Display *, XErrorEvent *);
static Bool otherwm, readin;

static void
cleanup()
{
	close(STDIN_FILENO);
	while(sel) {
		resize(sel, True, TopLeft);
		unmanage(sel);
	}
	if(dc.font.set)
		XFreeFontSet(dpy, dc.font.set);
	else
		XFreeFont(dpy, dc.font.xfont);
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	XDestroyWindow(dpy, barwin);
	XFreePixmap(dpy, dc.drawable);
	XFreeGC(dpy, dc.gc);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XSync(dpy, False);
}

static void
scan()
{
	unsigned int i, num;
	Window *wins, d1, d2;
	XWindowAttributes wa;

	wins = NULL;
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
	int status, format;
	unsigned long res, extra;
	Atom real;

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

/*
 * Startup Error handler to check if another window manager
 * is already running.
 */
static int
xerrorstart(Display *dsply, XErrorEvent *ee)
{
	otherwm = True;
	return -1;
}

/* extern */

char stext[1024];
Bool *seltag;
int screen, sx, sy, sw, sh, bx, by, bw, bh, mw;
unsigned int ntags, numlockmask;
Atom wmatom[WMLast], netatom[NetLast];
Bool running = True;
Bool issel = True;
Client *clients = NULL;
Client *sel = NULL;
Cursor cursor[CurLast];
Display *dpy;
DC dc = {0};
Window root, barwin;

int
getproto(Window w)
{
	int protos = 0;
	int i;
	long res;
	Atom *protocols;

	res = win_property(w, wmatom[WMProtocols], XA_ATOM, 20L,
			((unsigned char **)&protocols));
	if(res <= 0) {
		return protos;
	}
	for(i = 0; i < res; i++) {
		if(protocols[i] == wmatom[WMDelete])
			protos |= PROTODELWIN;
	}
	free((char *) protocols);
	return protos;
}

void
sendevent(Window w, Atom a, long value)
{
	XEvent e;

	e.type = ClientMessage;
	e.xclient.window = w;
	e.xclient.message_type = a;
	e.xclient.format = 32;
	e.xclient.data.l[0] = value;
	e.xclient.data.l[1] = CurrentTime;
	XSendEvent(dpy, w, False, NoEventMask, &e);
	XSync(dpy, False);
}

void
quit(Arg *arg)
{
	readin = running = False;
}

/*
 * There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's).  Other types of errors call Xlibs
 * default error handler, which calls exit().
 */
int
xerror(Display *dpy, XErrorEvent *ee)
{
	if(ee->error_code == BadWindow
	|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess))
		return 0;
	fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
		ee->request_code, ee->error_code);
	return xerrorxlib(dpy, ee); /* may call exit() */
}

int
main(int argc, char *argv[])
{
	int i, j, xfd;
	unsigned int mask;
	fd_set rd;
	Window w;
	XModifierKeymap *modmap;
	XSetWindowAttributes wa;

	if(argc == 2 && !strncmp("-v", argv[1], 3)) {
		fputs("dwm-"VERSION", (C)opyright MMVI Anselm R. Garbe\n", stdout);
		exit(EXIT_SUCCESS);
	}
	else if(argc != 1)
		eprint("usage: dwm [-v]\n");

	dpy = XOpenDisplay(0);
	if(!dpy)
		eprint("dwm: cannot open display\n");

	xfd = ConnectionNumber(dpy);
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);

	otherwm = False;
	XSetErrorHandler(xerrorstart);
	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, root, SubstructureRedirectMask);
	XSync(dpy, False);

	if(otherwm)
		eprint("dwm: another window manager is already running\n");

	XSync(dpy, False);
	XSetErrorHandler(NULL);
	xerrorxlib = XSetErrorHandler(xerror);
	XSync(dpy, False);

	/* init atoms */
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
	XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
			PropModeReplace, (unsigned char *) netatom, NetLast);

	/* init cursors */
	cursor[CurNormal] = XCreateFontCursor(dpy, XC_left_ptr);
	cursor[CurResize] = XCreateFontCursor(dpy, XC_sizing);
	cursor[CurMove] = XCreateFontCursor(dpy, XC_fleur);

	modmap = XGetModifierMapping(dpy);
	for (i = 0; i < 8; i++) {
		for (j = 0; j < modmap->max_keypermod; j++) {
			if(modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
		}
	}
	XFree(modmap);

	wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | EnterWindowMask | LeaveWindowMask;
	wa.cursor = cursor[CurNormal];
	XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &wa);

	grabkeys();
	initrregs();

	for(ntags = 0; tags[ntags]; ntags++);
	seltag = emallocz(sizeof(Bool) * ntags);
	seltag[DEFTAG] = True;

	/* style */
	dc.bg = getcolor(BGCOLOR);
	dc.fg = getcolor(FGCOLOR);
	dc.border = getcolor(BORDERCOLOR);
	setfont(FONT);

	sx = sy = 0;
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	mw = (sw * MASTERW) / 100;

	bx = by = 0;
	bw = sw;
	dc.h = bh = dc.font.height + 4;
	wa.override_redirect = 1;
	wa.background_pixmap = ParentRelative;
	wa.event_mask = ButtonPressMask | ExposureMask;
	barwin = XCreateWindow(dpy, root, bx, by, bw, bh, 0, DefaultDepth(dpy, screen),
			CopyFromParent, DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
	XDefineCursor(dpy, barwin, cursor[CurNormal]);
	XMapRaised(dpy, barwin);

	dc.drawable = XCreatePixmap(dpy, root, sw, bh, DefaultDepth(dpy, screen));
	dc.gc = XCreateGC(dpy, root, 0, 0);

	strcpy(stext, "dwm-"VERSION);
	drawstatus();

	issel = XQueryPointer(dpy, root, &w, &w, &i, &i, &i, &i, &mask);

	scan();

	/* main event loop, also reads status text from stdin */
	XSync(dpy, False);
	procevent();
	readin = True;
	while(running) {
		FD_ZERO(&rd);
		if(readin)
			FD_SET(STDIN_FILENO, &rd);
		FD_SET(xfd, &rd);
		i = select(xfd + 1, &rd, NULL, NULL, NULL);
		if(i == -1 && errno == EINTR)
			continue;
		if(i < 0)
			eprint("select failed\n");
		else if(i > 0) {
			if(readin && FD_ISSET(STDIN_FILENO, &rd)) {
				readin = NULL != fgets(stext, sizeof(stext), stdin);
				if(readin)
					stext[strlen(stext) - 1] = 0;
				else 
					strcpy(stext, "broken pipe");
				drawstatus();
			}
		}
		procevent();
	}
	cleanup();
	XCloseDisplay(dpy);

	return 0;
}
