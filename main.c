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
int bh, bmw, screen, sx, sy, sw, sh, wax, way, waw, wah;
unsigned int ntags, numlockmask;
Atom wmatom[WMLast], netatom[NetLast];
Bool running = True;
Bool *seltag;
Bool selscreen = True;
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
		if(stack->isbanned)
			XMoveWindow(dpy, stack->win, stack->x, stack->y);
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

static unsigned int
textnw(const char *text, unsigned int len) {
	XRectangle r;

	if(dc.font.set) {
		XmbTextExtents(dc.font.set, text, len, NULL, &r);
		return r.width;
	}
	return XTextWidth(dc.font.xfont, text, len);
}

static void
drawtext(const char *text, unsigned long col[ColLast], Bool filledsquare, Bool emptysquare) {
	int x, y, w, h;
	static char buf[256];
	unsigned int len, olen;
	XGCValues gcv;
	XRectangle r = { dc.x, dc.y, dc.w, dc.h };

	XSetForeground(dpy, dc.gc, col[ColBG]);
	XFillRectangles(dpy, dc.drawable, dc.gc, &r, 1);
	if(!text)
		return;
	w = 0;
	olen = len = strlen(text);
	if(len >= sizeof buf)
		len = sizeof buf - 1;
	memcpy(buf, text, len);
	buf[len] = 0;
	h = dc.font.ascent + dc.font.descent;
	y = dc.y + (dc.h / 2) - (h / 2) + dc.font.ascent;
	x = dc.x + (h / 2);
	/* shorten text if necessary */
	while(len && (w = textnw(buf, len)) > dc.w - h)
		buf[--len] = 0;
	if(len < olen) {
		if(len > 1)
			buf[len - 1] = '.';
		if(len > 2)
			buf[len - 2] = '.';
		if(len > 3)
			buf[len - 3] = '.';
	}
	if(w > dc.w)
		return; /* too long */
	gcv.foreground = col[ColFG];
	if(dc.font.set) {
		XChangeGC(dpy, dc.gc, GCForeground, &gcv);
		XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, x, y, buf, len);
	}
	else {
		gcv.font = dc.font.xfont->fid;
		XChangeGC(dpy, dc.gc, GCForeground | GCFont, &gcv);
		XDrawString(dpy, dc.drawable, dc.gc, x, y, buf, len);
	}
	x = (h + 2) / 4;
	r.x = dc.x + 1;
	r.y = dc.y + 1;
	if(filledsquare) {
		r.width = r.height = x + 1;
		XFillRectangles(dpy, dc.drawable, dc.gc, &r, 1);
	}
	else if(emptysquare) {
		r.width = r.height = x;
		XDrawRectangles(dpy, dc.drawable, dc.gc, &r, 1);
	}
}

static unsigned long
getcolor(const char *colstr) {
	Colormap cmap = DefaultColormap(dpy, screen);
	XColor color;

	if(!XAllocNamedColor(dpy, cmap, colstr, &color, &color))
		eprint("error, cannot allocate color '%s'\n", colstr);
	return color.pixel;
}

static Bool
isoccupied(unsigned int t) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(c->tags[t])
			return True;
	return False;
}

static void
scan(void) {
	unsigned int i, num;
	Window *wins, d1, d2;
	XWindowAttributes wa;

	wins = NULL;
	if(XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for(i = 0; i < num; i++) {
			if(!XGetWindowAttributes(dpy, wins[i], &wa)
			|| wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if(wa.map_state == IsViewable)
				manage(wins[i], &wa);
		}
	}
	if(wins)
		XFree(wins);
}

static void
setfont(const char *fontstr) {
	char *def, **missing;
	int i, n;

	missing = NULL;
	if(dc.font.set)
		XFreeFontSet(dpy, dc.font.set);
	dc.font.set = XCreateFontSet(dpy, fontstr, &missing, &n, &def);
	if(missing) {
		while(n--)
			fprintf(stderr, "missing fontset: %s\n", missing[n]);
		XFreeStringList(missing);
	}
	if(dc.font.set) {
		XFontSetExtents *font_extents;
		XFontStruct **xfonts;
		char **font_names;
		dc.font.ascent = dc.font.descent = 0;
		font_extents = XExtentsOfFontSet(dc.font.set);
		n = XFontsOfFontSet(dc.font.set, &xfonts, &font_names);
		for(i = 0, dc.font.ascent = 0, dc.font.descent = 0; i < n; i++) {
			if(dc.font.ascent < (*xfonts)->ascent)
				dc.font.ascent = (*xfonts)->ascent;
			if(dc.font.descent < (*xfonts)->descent)
				dc.font.descent = (*xfonts)->descent;
			xfonts++;
		}
	}
	else {
		if(dc.font.xfont)
			XFreeFont(dpy, dc.font.xfont);
		dc.font.xfont = NULL;
		if(!(dc.font.xfont = XLoadQueryFont(dpy, fontstr)))
			eprint("error, cannot load font: '%s'\n", fontstr);
		dc.font.ascent = dc.font.xfont->ascent;
		dc.font.descent = dc.font.xfont->descent;
	}
	dc.font.height = dc.font.ascent + dc.font.descent;
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
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
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
	for (i = 0; i < 8; i++)
		for (j = 0; j < modmap->max_keypermod; j++) {
			if(modmap->modifiermap[i * modmap->max_keypermod + j]
					== XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
		}
	XFreeModifiermap(modmap);
	/* select for events */
	wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask
		| EnterWindowMask | LeaveWindowMask;
	wa.cursor = cursor[CurNormal];
	XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &wa);
	grabkeys();
	compileregexps();
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
	bmw = textw(TILESYMBOL) > textw(FLOATSYMBOL) ? textw(TILESYMBOL) : textw(FLOATSYMBOL);
	/* bar */
	dc.h = bh = dc.font.height + 2;
	wa.override_redirect = 1;
	wa.background_pixmap = ParentRelative;
	wa.event_mask = ButtonPressMask | ExposureMask;
	barwin = XCreateWindow(dpy, root, sx, sy + (TOPBAR ? 0 : sh - bh), sw, bh, 0,
			DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
	XDefineCursor(dpy, barwin, cursor[CurNormal]);
	XMapRaised(dpy, barwin);
	strcpy(stext, "dwm-"VERSION);
	/* windowarea */
	wax = sx;
	way = sy + (TOPBAR ? bh : 0);
	wah = sh - bh;
	waw = sw;
	/* pixmap for everything */
	dc.drawable = XCreatePixmap(dpy, root, sw, bh, DefaultDepth(dpy, screen));
	dc.gc = XCreateGC(dpy, root, 0, 0);
	XSetLineAttributes(dpy, dc.gc, 1, LineSolid, CapButt, JoinMiter);
	/* multihead support */
	selscreen = XQueryPointer(dpy, root, &w, &w, &i, &i, &i, &i, &mask);
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

void
drawstatus(void) {
	int i, x;

	dc.x = dc.y = 0;
	for(i = 0; i < ntags; i++) {
		dc.w = textw(tags[i]);
		if(seltag[i])
			drawtext(tags[i], dc.sel, sel && sel->tags[i], isoccupied(i));
		else
			drawtext(tags[i], dc.norm, sel && sel->tags[i], isoccupied(i));
		dc.x += dc.w;
	}
	dc.w = bmw;
	drawtext(arrange == dofloat ? FLOATSYMBOL : TILESYMBOL, dc.norm, False, False);
	x = dc.x + dc.w;
	dc.w = textw(stext);
	dc.x = sw - dc.w;
	if(dc.x < x) {
		dc.x = x;
		dc.w = sw - x;
	}
	drawtext(stext, dc.norm, False, False);
	if((dc.w = dc.x - x) > bh) {
		dc.x = x;
		drawtext(sel ? sel->name : NULL, sel ? dc.sel : dc.norm, False, False);
	}
	XCopyArea(dpy, dc.drawable, barwin, dc.gc, 0, 0, sw, bh, 0, 0);
	XSync(dpy, False);
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

unsigned int
textw(const char *text) {
	return textnw(text, strlen(text)) + dc.font.height;
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
				for(; p >= stext && *p != '\n'; --p);
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
