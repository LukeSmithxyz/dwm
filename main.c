/* (C)opyright MMVI-MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "dwm.h"
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>

/* extern */

char stext[256];
Bool *seltag;
int bx, by, bw, bh, bmw, masterd, screen, sx, sy, sw, sh, wax, way, waw, wah;
unsigned int master, nmaster, ntags, numlockmask;
Atom wmatom[WMLast], netatom[NetLast];
Bool running = True;
Bool issel = True;
Client *clients = NULL;
Client *sel = NULL;
Client *stack = NULL;
Cursor cursor[CurLast];
Display *dpy;
DC dc = {0};
Window root, barwin;

/* static */

static int (*xerrorxlib)(Display *, XErrorEvent *);
static Bool otherwm, readin;

static void
cleanup(void) {
	close(STDIN_FILENO);
	while(stack) {
		resize(stack, True, TopLeft);
		unmanage(stack);
	}
	if(dc.font.set)
		XFreeFontSet(dpy, dc.font.set);
	else
		XFreeFont(dpy, dc.font.xfont);
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	XFreePixmap(dpy, dc.drawable);
	XFreeGC(dpy, dc.gc);
	XDestroyWindow(dpy, barwin);
	XFreeCursor(dpy, cursor[CurNormal]);
	XFreeCursor(dpy, cursor[CurResize]);
	XFreeCursor(dpy, cursor[CurMove]);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XSync(dpy, False);
	free(seltag);
}

static void
scan(void) {
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

static void
setup(void) {
	int i, j;
	unsigned int mask;
	Window w;
	XModifierKeymap *modmap;
	XSetWindowAttributes wa;

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
	/* init modifier map */
	numlockmask = 0;
	modmap = XGetModifierMapping(dpy);
	for (i = 0; i < 8; i++) {
		for (j = 0; j < modmap->max_keypermod; j++) {
			if(modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
		}
	}
	XFreeModifiermap(modmap);
	/* select for events */
	wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask
		| EnterWindowMask | LeaveWindowMask;
	wa.cursor = cursor[CurNormal];
	XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &wa);
	grabkeys();
	initrregs();
	for(ntags = 0; tags[ntags]; ntags++);
	seltag = emallocz(sizeof(Bool) * ntags);
	seltag[0] = True;
	/* style */
	dc.norm[ColBorder] = getcolor(NORMBORDERCOLOR);
	dc.norm[ColBG] = getcolor(NORMBGCOLOR);
	dc.norm[ColFG] = getcolor(NORMFGCOLOR);
	dc.sel[ColBorder] = getcolor(SELBORDERCOLOR);
	dc.sel[ColBG] = getcolor(SELBGCOLOR);
	dc.sel[ColFG] = getcolor(SELFGCOLOR);
	setfont(FONT);
	/* geometry */
	sx = sy = 0;
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	master = MASTER;
	nmaster = NMASTER;
	bmw = textw(TILESYMBOL) > textw(FLOATSYMBOL) ? textw(TILESYMBOL) : textw(FLOATSYMBOL);
	/* bar */
	bx = sx;
	by = sy;
	bw = sw;
	dc.h = bh = dc.font.height + 2;
	wa.override_redirect = 1;
	wa.background_pixmap = ParentRelative;
	wa.event_mask = ButtonPressMask | ExposureMask;
	barwin = XCreateWindow(dpy, root, bx, by, bw, bh, 0, DefaultDepth(dpy, screen),
			CopyFromParent, DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
	XDefineCursor(dpy, barwin, cursor[CurNormal]);
	XMapRaised(dpy, barwin);
	strcpy(stext, "dwm-"VERSION);
	/* windowarea */
	wax = sx;
	way = sy + bh;
	wah = sh - bh;
	waw = sw;
	/* pixmap for everything */
	dc.drawable = XCreatePixmap(dpy, root, sw, bh, DefaultDepth(dpy, screen));
	dc.gc = XCreateGC(dpy, root, 0, 0);
	XSetLineAttributes(dpy, dc.gc, 1, LineSolid, CapButt, JoinMiter);
	/* multihead support */
	issel = XQueryPointer(dpy, root, &w, &w, &i, &i, &i, &i, &mask);
}

/*
 * Startup Error handler to check if another window manager
 * is already running.
 */
static int
xerrorstart(Display *dsply, XErrorEvent *ee) {
	otherwm = True;
	return -1;
}

/* extern */

int
getproto(Window w) {
	int i, format, protos, status;
	unsigned long extra, res;
	Atom *protocols, real;

	protos = 0;
	status = XGetWindowProperty(dpy, w, wmatom[WMProtocols], 0L, 20L, False,
			XA_ATOM, &real, &format, &res, &extra, (unsigned char **)&protocols);
	if(status != Success || protocols == 0)
		return protos;
	for(i = 0; i < res; i++)
		if(protocols[i] == wmatom[WMDelete])
			protos |= PROTODELWIN;
	free(protocols);
	return protos;
}

void
sendevent(Window w, Atom a, long value) {
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
quit(Arg *arg) {
	readin = running = False;
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's).  Other types of errors call Xlibs
 * default error handler, which may call exit.
 */
int
xerror(Display *dpy, XErrorEvent *ee) {
	if(ee->error_code == BadWindow
	|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
	fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
		ee->request_code, ee->error_code);
	return xerrorxlib(dpy, ee); /* may call exit */
}

int
main(int argc, char *argv[]) {
	char *p;
	int r, xfd;
	fd_set rd;

	if(argc == 2 && !strncmp("-v", argv[1], 3)) {
		fputs("dwm-"VERSION", (C)opyright MMVI-MMVII Anselm R. Garbe\n", stdout);
		exit(EXIT_SUCCESS);
	}
	else if(argc != 1)
		eprint("usage: dwm [-v]\n");
	setlocale(LC_CTYPE, "");
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
	setup();
	drawstatus();
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
		if(select(xfd + 1, &rd, NULL, NULL, NULL) == -1) {
			if(errno == EINTR)
				continue;
			eprint("select failed\n");
		}
		if(FD_ISSET(STDIN_FILENO, &rd)) {
			switch(r = read(STDIN_FILENO, stext, sizeof stext - 1)) {
			case -1:
				strncpy(stext, strerror(errno), sizeof stext - 1);
				stext[sizeof stext - 1] = '\0';
				readin = False;
				break;
			case 0:
				strncpy(stext, "EOF", 4);
				readin = False;
				break;
			default:
				for(stext[r] = '\0', p = stext + strlen(stext) - 1; p >= stext && *p == '\n'; *p-- = '\0');
				for(p = stext + strlen(stext) - 1; p >= stext && *p != '\n'; --p);
				if(p > stext)
					strncpy(stext, p + 1, sizeof stext);
			}
			drawstatus();
		}
		if(FD_ISSET(xfd, &rd))
			procevent();
	}
	cleanup();
	XCloseDisplay(dpy);
	return 0;
}
