/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance.  Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * Calls to fetch an X event from the event queue are blocking.  Due reading
 * status text from standard input, a select()-driven main loop has been
 * implemented which selects for reads on the X connection and STDIN_FILENO to
 * handle all data smoothly. The event handlers of dwm are organized in an
 * array which is accessed whenever a new event has been fetched. This allows
 * event dispatching in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag.  Clients are organized in a global
 * linked client list, the focus history is remembered through a global
 * stack list. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

/* macros */
#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask))
#define INRECT(X,Y,RX,RY,RW,RH) ((X) >= (RX) && (X) < (RX) + (RW) && (Y) >= (RY) && (Y) < (RY) + (RH))
#define ISVISIBLE(x)            (x->tags & tagset[seltags])
#define LENGTH(x)               (sizeof x / sizeof x[0])
#define MAX(a, b)               ((a) > (b) ? (a) : (b))
#define MIN(a, b)               ((a) < (b) ? (a) : (b))
#define MAXTAGLEN               16
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define WIDTH(x)                ((x)->w + 2 * (x)->bw)
#define HEIGHT(x)               ((x)->h + 2 * (x)->bw)
#define TAGMASK                 ((int)((1LL << LENGTH(tags)) - 1))
#define TEXTW(x)                (textnw(x, strlen(x)) + dc.font.height)

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast };        /* cursor */
enum { ColBorder, ColFG, ColBG, ColLast };              /* color */
enum { NetSupported, NetWMName, NetLast };              /* EWMH atoms */
enum { WMProtocols, WMDelete, WMState, WMLast };        /* default atoms */
enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast };             /* clicks */

typedef union {
	int i;
	unsigned int ui;
	float f;
	void *v;
} Arg;

typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const Arg *arg);
	const Arg arg;
} Button;

typedef struct Client Client;
struct Client {
	char name[256];
	float mina, maxa;
	int x, y, w, h;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int bw, oldbw;
	unsigned int tags;
	Bool isfixed, isfloating, isurgent;
	Client *next;
	Client *snext;
	Window win;
};

typedef struct {
	int x, y, w, h;
	unsigned long norm[ColLast];
	unsigned long sel[ColLast];
	Drawable drawable;
	GC gc;
	struct {
		int ascent;
		int descent;
		int height;
		XFontSet set;
		XFontStruct *xfont;
	} font;
} DC; /* draw context */

typedef struct {
	unsigned int mod;
	KeySym keysym;
	void (*func)(const Arg *);
	const Arg arg;
} Key;

typedef struct {
	const char *symbol;
	void (*arrange)(void);
} Layout;

typedef struct {
	const char *class;
	const char *instance;
	const char *title;
	unsigned int tags;
	Bool isfloating;
} Rule;

/* function declarations */
static void applyrules(Client *c);
static void arrange(void);
static void attach(Client *c);
static void attachstack(Client *c);
static void buttonpress(XEvent *e);
static void checkotherwm(void);
static void cleanup(void);
static void clearurgent(void);
static void configure(Client *c);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static void destroynotify(XEvent *e);
static void detach(Client *c);
static void detachstack(Client *c);
static void die(const char *errstr, ...);
static void drawbar(void);
static void drawsquare(Bool filled, Bool empty, Bool invert, unsigned long col[ColLast]);
static void drawtext(const char *text, unsigned long col[ColLast], Bool invert);
static void enternotify(XEvent *e);
static void expose(XEvent *e);
static void focus(Client *c);
static void focusin(XEvent *e);
static void focusstack(const Arg *arg);
static Client *getclient(Window w);
static unsigned long getcolor(const char *colstr);
static long getstate(Window w);
static Bool gettextprop(Window w, Atom atom, char *text, unsigned int size);
static void grabbuttons(Client *c, Bool focused);
static void grabkeys(void);
static void initfont(const char *fontstr);
static Bool isprotodel(Client *c);
static void keypress(XEvent *e);
static void killclient(const Arg *arg);
static void manage(Window w, XWindowAttributes *wa);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void monocle(void);
static void movemouse(const Arg *arg);
static Client *nexttiled(Client *c);
static void propertynotify(XEvent *e);
static void quit(const Arg *arg);
static void resize(Client *c, int x, int y, int w, int h, Bool sizehints);
static void resizemouse(const Arg *arg);
static void restack(void);
static void run(void);
static void scan(void);
static void setclientstate(Client *c, long state);
static void setlayout(const Arg *arg);
static void setmfact(const Arg *arg);
static void setup(void);
static void showhide(Client *c);
static void sigchld(int signal);
static void spawn(const Arg *arg);
static void tag(const Arg *arg);
static int textnw(const char *text, unsigned int len);
static void tile(void);
static void togglebar(const Arg *arg);
static void togglefloating(const Arg *arg);
static void toggletag(const Arg *arg);
static void toggleview(const Arg *arg);
static void unmanage(Client *c);
static void unmapnotify(XEvent *e);
static void updatebar(void);
static void updategeom(void);
static void updatenumlockmask(void);
static void updatesizehints(Client *c);
static void updatetitle(Client *c);
static void updatewmhints(Client *c);
static void view(const Arg *arg);
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);
static void zoom(const Arg *arg);

/* variables */
static char stext[256];
static int screen;
static int sx, sy, sw, sh; /* X display screen geometry x, y, width, height */ 
static int by, bh, blw;    /* bar geometry y, height and layout symbol width */
static int wx, wy, ww, wh; /* window area geometry x, y, width, height, bar excluded */
static unsigned int seltags = 0, sellt = 0;
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress] = buttonpress,
	[ConfigureRequest] = configurerequest,
	[ConfigureNotify] = configurenotify,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = enternotify,
	[Expose] = expose,
	[FocusIn] = focusin,
	[KeyPress] = keypress,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[PropertyNotify] = propertynotify,
	[UnmapNotify] = unmapnotify
};
static Atom wmatom[WMLast], netatom[NetLast];
static Bool otherwm;
static Bool running = True;
static Client *clients = NULL;
static Client *sel = NULL;
static Client *stack = NULL;
static Cursor cursor[CurLast];
static Display *dpy;
static DC dc;
static Layout *lt[] = { NULL, NULL };
static Window root, barwin;
/* configuration, allows nested code to access above variables */
#include "config.h"

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags { char limitexceeded[sizeof(unsigned int) * 8 < LENGTH(tags) ? -1 : 1]; };

/* function implementations */
void
applyrules(Client *c) {
	unsigned int i;
	Rule *r;
	XClassHint ch = { 0 };

	/* rule matching */
	if(XGetClassHint(dpy, c->win, &ch)) {
		for(i = 0; i < LENGTH(rules); i++) {
			r = &rules[i];
			if((!r->title || strstr(c->name, r->title))
			&& (!r->class || (ch.res_class && strstr(ch.res_class, r->class)))
			&& (!r->instance || (ch.res_name && strstr(ch.res_name, r->instance)))) {
				c->isfloating = r->isfloating;
				c->tags |= r->tags & TAGMASK;
			}
		}
		if(ch.res_class)
			XFree(ch.res_class);
		if(ch.res_name)
			XFree(ch.res_name);
	}
	if(!c->tags)
		c->tags = tagset[seltags];
}

void
arrange(void) {
	showhide(stack);
	focus(NULL);
	if(lt[sellt]->arrange)
		lt[sellt]->arrange();
	restack();
}

void
attach(Client *c) {
	c->next = clients;
	clients = c;
}

void
attachstack(Client *c) {
	c->snext = stack;
	stack = c;
}

void
buttonpress(XEvent *e) {
	unsigned int i, x, click;
	Arg arg = {0};
	Client *c;
	XButtonPressedEvent *ev = &e->xbutton;

	click = ClkRootWin;
	if(ev->window == barwin) {
		i = x = 0;
		do x += TEXTW(tags[i]); while(ev->x >= x && ++i < LENGTH(tags));
		if(i < LENGTH(tags)) {
			click = ClkTagBar;
			arg.ui = 1 << i;
		}
		else if(ev->x < x + blw)
			click = ClkLtSymbol;
		else if(ev->x > wx + ww - TEXTW(stext))
			click = ClkStatusText;
		else
			click = ClkWinTitle;
	}
	else if((c = getclient(ev->window))) {
		focus(c);
		click = ClkClientWin;
	}

	for(i = 0; i < LENGTH(buttons); i++)
		if(click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
		   && CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
			buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
}

void
checkotherwm(void) {
	otherwm = False;
	xerrorxlib = XSetErrorHandler(xerrorstart);

	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
	XSync(dpy, False);
	if(otherwm)
		die("dwm: another window manager is already running\n");
	XSetErrorHandler(xerror);
	XSync(dpy, False);
}

void
cleanup(void) {
	Arg a = {.ui = ~0};
	Layout foo = { "", NULL };

	close(STDIN_FILENO);
	view(&a);
	lt[sellt] = &foo;
	while(stack)
		unmanage(stack);
	if(dc.font.set)
		XFreeFontSet(dpy, dc.font.set);
	else
		XFreeFont(dpy, dc.font.xfont);
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	XFreePixmap(dpy, dc.drawable);
	XFreeGC(dpy, dc.gc);
	XFreeCursor(dpy, cursor[CurNormal]);
	XFreeCursor(dpy, cursor[CurResize]);
	XFreeCursor(dpy, cursor[CurMove]);
	XDestroyWindow(dpy, barwin);
	XSync(dpy, False);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
}

void
clearurgent(void) {
	XWMHints *wmh;
	Client *c;

	for(c = clients; c; c = c->next)
		if(ISVISIBLE(c) && c->isurgent) {
			c->isurgent = False;
			if (!(wmh = XGetWMHints(dpy, c->win)))
				continue;

			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dpy, c->win, wmh);
			XFree(wmh);
		}
}

void
configure(Client *c) {
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = c->bw;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void
configurenotify(XEvent *e) {
	XConfigureEvent *ev = &e->xconfigure;

	if(ev->window == root && (ev->width != sw || ev->height != sh)) {
		sw = ev->width;
		sh = ev->height;
		updategeom();
		updatebar();
		arrange();
	}
}

void
configurerequest(XEvent *e) {
	Client *c;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;

	if((c = getclient(ev->window))) {
		if(ev->value_mask & CWBorderWidth)
			c->bw = ev->border_width;
		else if(c->isfloating || !lt[sellt]->arrange) {
			if(ev->value_mask & CWX)
				c->x = sx + ev->x;
			if(ev->value_mask & CWY)
				c->y = sy + ev->y;
			if(ev->value_mask & CWWidth)
				c->w = ev->width;
			if(ev->value_mask & CWHeight)
				c->h = ev->height;
			if((c->x - sx + c->w) > sw && c->isfloating)
				c->x = sx + (sw / 2 - c->w / 2); /* center in x direction */
			if((c->y - sy + c->h) > sh && c->isfloating)
				c->y = sy + (sh / 2 - c->h / 2); /* center in y direction */
			if((ev->value_mask & (CWX|CWY)) && !(ev->value_mask & (CWWidth|CWHeight)))
				configure(c);
			if(ISVISIBLE(c))
				XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
		}
		else
			configure(c);
	}
	else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
	}
	XSync(dpy, False);
}

void
destroynotify(XEvent *e) {
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if((c = getclient(ev->window)))
		unmanage(c);
}

void
detach(Client *c) {
	Client **tc;

	for(tc = &clients; *tc && *tc != c; tc = &(*tc)->next);
	*tc = c->next;
}

void
detachstack(Client *c) {
	Client **tc;

	for(tc = &stack; *tc && *tc != c; tc = &(*tc)->snext);
	*tc = c->snext;
}

void
die(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void
drawbar(void) {
	int x;
	unsigned int i, occ = 0, urg = 0;
	unsigned long *col;
	Client *c;

	for(c = clients; c; c = c->next) {
		occ |= c->tags;
		if(c->isurgent)
			urg |= c->tags;
	}

	dc.x = 0;
	for(i = 0; i < LENGTH(tags); i++) {
		dc.w = TEXTW(tags[i]);
		col = tagset[seltags] & 1 << i ? dc.sel : dc.norm;
		drawtext(tags[i], col, urg & 1 << i);
		drawsquare(sel && sel->tags & 1 << i, occ & 1 << i, urg & 1 << i, col);
		dc.x += dc.w;
	}
	if(blw > 0) {
		dc.w = blw;
		drawtext(lt[sellt]->symbol, dc.norm, False);
		x = dc.x + dc.w;
	}
	else
		x = dc.x;
	dc.w = TEXTW(stext);
	dc.x = ww - dc.w;
	if(dc.x < x) {
		dc.x = x;
		dc.w = ww - x;
	}
	drawtext(stext, dc.norm, False);
	if((dc.w = dc.x - x) > bh) {
		dc.x = x;
		if(sel) {
			drawtext(sel->name, dc.sel, False);
			drawsquare(sel->isfixed, sel->isfloating, False, dc.sel);
		}
		else
			drawtext(NULL, dc.norm, False);
	}
	XCopyArea(dpy, dc.drawable, barwin, dc.gc, 0, 0, ww, bh, 0, 0);
	XSync(dpy, False);
}

void
drawsquare(Bool filled, Bool empty, Bool invert, unsigned long col[ColLast]) {
	int x;
	XGCValues gcv;
	XRectangle r = { dc.x, dc.y, dc.w, dc.h };

	gcv.foreground = col[invert ? ColBG : ColFG];
	XChangeGC(dpy, dc.gc, GCForeground, &gcv);
	x = (dc.font.ascent + dc.font.descent + 2) / 4;
	r.x = dc.x + 1;
	r.y = dc.y + 1;
	if(filled) {
		r.width = r.height = x + 1;
		XFillRectangles(dpy, dc.drawable, dc.gc, &r, 1);
	}
	else if(empty) {
		r.width = r.height = x;
		XDrawRectangles(dpy, dc.drawable, dc.gc, &r, 1);
	}
}

void
drawtext(const char *text, unsigned long col[ColLast], Bool invert) {
	char buf[256];
	int i, x, y, h, len, olen;
	XRectangle r = { dc.x, dc.y, dc.w, dc.h };

	XSetForeground(dpy, dc.gc, col[invert ? ColFG : ColBG]);
	XFillRectangles(dpy, dc.drawable, dc.gc, &r, 1);
	if(!text)
		return;
	olen = strlen(text);
	h = dc.font.ascent + dc.font.descent;
	y = dc.y + (dc.h / 2) - (h / 2) + dc.font.ascent;
	x = dc.x + (h / 2);
	/* shorten text if necessary */
	for(len = MIN(olen, sizeof buf); len && textnw(text, len) > dc.w - h; len--);
	if(!len)
		return;
	memcpy(buf, text, len);
	if(len < olen)
		for(i = len; i && i > len - 3; buf[--i] = '.');
	XSetForeground(dpy, dc.gc, col[invert ? ColBG : ColFG]);
	if(dc.font.set)
		XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, x, y, buf, len);
	else
		XDrawString(dpy, dc.drawable, dc.gc, x, y, buf, len);
}

void
enternotify(XEvent *e) {
	Client *c;
	XCrossingEvent *ev = &e->xcrossing;

	if((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
		return;
	if((c = getclient(ev->window)))
		focus(c);
	else
		focus(NULL);
}

void
expose(XEvent *e) {
	XExposeEvent *ev = &e->xexpose;

	if(ev->count == 0 && (ev->window == barwin))
		drawbar();
}

void
focus(Client *c) {
	if(!c || !ISVISIBLE(c))
		for(c = stack; c && !ISVISIBLE(c); c = c->snext);
	if(sel && sel != c) {
		grabbuttons(sel, False);
		XSetWindowBorder(dpy, sel->win, dc.norm[ColBorder]);
	}
	if(c) {
		detachstack(c);
		attachstack(c);
		grabbuttons(c, True);
		XSetWindowBorder(dpy, c->win, dc.sel[ColBorder]);
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
	}
	else
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
	sel = c;
	drawbar();
}

void
focusin(XEvent *e) { /* there are some broken focus acquiring clients */
	XFocusChangeEvent *ev = &e->xfocus;

	if(sel && ev->window != sel->win)
		XSetInputFocus(dpy, sel->win, RevertToPointerRoot, CurrentTime);
}

void
focusstack(const Arg *arg) {
	Client *c = NULL, *i;

	if(!sel)
		return;
	if (arg->i > 0) {
		for(c = sel->next; c && !ISVISIBLE(c); c = c->next);
		if(!c)
			for(c = clients; c && !ISVISIBLE(c); c = c->next);
	}
	else {
		for(i = clients; i != sel; i = i->next)
			if(ISVISIBLE(i))
				c = i;
		if(!c)
			for(; i; i = i->next)
				if(ISVISIBLE(i))
					c = i;
	}
	if(c) {
		focus(c);
		restack();
	}
}

Client *
getclient(Window w) {
	Client *c;

	for(c = clients; c && c->win != w; c = c->next);
	return c;
}

unsigned long
getcolor(const char *colstr) {
	Colormap cmap = DefaultColormap(dpy, screen);
	XColor color;

	if(!XAllocNamedColor(dpy, cmap, colstr, &color, &color))
		die("error, cannot allocate color '%s'\n", colstr);
	return color.pixel;
}

long
getstate(Window w) {
	int format, status;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;

	status = XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState],
			&real, &format, &n, &extra, (unsigned char **)&p);
	if(status != Success)
		return -1;
	if(n != 0)
		result = *p;
	XFree(p);
	return result;
}

Bool
gettextprop(Window w, Atom atom, char *text, unsigned int size) {
	char **list = NULL;
	int n;
	XTextProperty name;

	if(!text || size == 0)
		return False;
	text[0] = '\0';
	XGetTextProperty(dpy, w, &name, atom);
	if(!name.nitems)
		return False;
	if(name.encoding == XA_STRING)
		strncpy(text, (char *)name.value, size - 1);
	else {
		if(XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success
		&& n > 0 && *list) {
			strncpy(text, *list, size - 1);
			XFreeStringList(list);
		}
	}
	text[size - 1] = '\0';
	XFree(name.value);
	return True;
}

void
grabbuttons(Client *c, Bool focused) {
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		if(focused) {
			for(i = 0; i < LENGTH(buttons); i++)
				if(buttons[i].click == ClkClientWin)
					for(j = 0; j < LENGTH(modifiers); j++)
						XGrabButton(dpy, buttons[i].button, buttons[i].mask | modifiers[j], c->win, False, BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
		} else
			XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
			            BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
	}
}

void
grabkeys(void) {
	updatenumlockmask();
	{ /* grab keys */
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		KeyCode code;

		XUngrabKey(dpy, AnyKey, AnyModifier, root);
		for(i = 0; i < LENGTH(keys); i++) {
			if((code = XKeysymToKeycode(dpy, keys[i].keysym)))
				for(j = 0; j < LENGTH(modifiers); j++)
					XGrabKey(dpy, code, keys[i].mod | modifiers[j], root,
						 True, GrabModeAsync, GrabModeAsync);
		}
	}
}

void
initfont(const char *fontstr) {
	char *def, **missing;
	int i, n;

	missing = NULL;
	dc.font.set = XCreateFontSet(dpy, fontstr, &missing, &n, &def);
	if(missing) {
		while(n--)
			fprintf(stderr, "dwm: missing fontset: %s\n", missing[n]);
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
			dc.font.ascent = MAX(dc.font.ascent, (*xfonts)->ascent);
			dc.font.descent = MAX(dc.font.descent,(*xfonts)->descent);
			xfonts++;
		}
	}
	else {
		if(!(dc.font.xfont = XLoadQueryFont(dpy, fontstr))
		&& !(dc.font.xfont = XLoadQueryFont(dpy, "fixed")))
			die("error, cannot load font: '%s'\n", fontstr);
		dc.font.ascent = dc.font.xfont->ascent;
		dc.font.descent = dc.font.xfont->descent;
	}
	dc.font.height = dc.font.ascent + dc.font.descent;
}

Bool
isprotodel(Client *c) {
	int i, n;
	Atom *protocols;
	Bool ret = False;

	if(XGetWMProtocols(dpy, c->win, &protocols, &n)) {
		for(i = 0; !ret && i < n; i++)
			if(protocols[i] == wmatom[WMDelete])
				ret = True;
		XFree(protocols);
	}
	return ret;
}

void
keypress(XEvent *e) {
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ev;

	ev = &e->xkey;
	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
	for(i = 0; i < LENGTH(keys); i++)
		if(keysym == keys[i].keysym
		   && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
		   && keys[i].func)
			keys[i].func(&(keys[i].arg));
}

void
killclient(const Arg *arg) {
	XEvent ev;

	if(!sel)
		return;
	if(isprotodel(sel)) {
		ev.type = ClientMessage;
		ev.xclient.window = sel->win;
		ev.xclient.message_type = wmatom[WMProtocols];
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = wmatom[WMDelete];
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(dpy, sel->win, False, NoEventMask, &ev);
	}
	else
		XKillClient(dpy, sel->win);
}

void
manage(Window w, XWindowAttributes *wa) {
	static Client cz;
	Client *c, *t = NULL;
	Window trans = None;
	XWindowChanges wc;

	if(!(c = malloc(sizeof(Client))))
		die("fatal: could not malloc() %u bytes\n", sizeof(Client));
	*c = cz;
	c->win = w;

	/* geometry */
	c->x = wa->x;
	c->y = wa->y;
	c->w = wa->width;
	c->h = wa->height;
	c->oldbw = wa->border_width;
	if(c->w == sw && c->h == sh) {
		c->x = sx;
		c->y = sy;
		c->bw = 0;
	}
	else {
		if(c->x + WIDTH(c) > sx + sw)
			c->x = sx + sw - WIDTH(c);
		if(c->y + HEIGHT(c) > sy + sh)
			c->y = sy + sh - HEIGHT(c);
		c->x = MAX(c->x, sx);
		/* only fix client y-offset, if the client center might cover the bar */
		c->y = MAX(c->y, ((by == 0) && (c->x + (c->w / 2) >= wx) && (c->x + (c->w / 2) < wx + ww)) ? bh : sy);
		c->bw = borderpx;
	}

	wc.border_width = c->bw;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, w, dc.norm[ColBorder]);
	configure(c); /* propagates border_width, if size doesn't change */
	updatesizehints(c);
	XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
	grabbuttons(c, False);
	updatetitle(c);
	if(XGetTransientForHint(dpy, w, &trans))
		t = getclient(trans);
	if(t)
		c->tags = t->tags;
	else
		applyrules(c);
	if(!c->isfloating)
		c->isfloating = trans != None || c->isfixed;
	if(c->isfloating)
		XRaiseWindow(dpy, c->win);
	attach(c);
	attachstack(c);
	XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h); /* some windows require this */
	XMapWindow(dpy, c->win);
	setclientstate(c, NormalState);
	arrange();
}

void
mappingnotify(XEvent *e) {
	XMappingEvent *ev = &e->xmapping;

	XRefreshKeyboardMapping(ev);
	if(ev->request == MappingKeyboard)
		grabkeys();
}

void
maprequest(XEvent *e) {
	static XWindowAttributes wa;
	XMapRequestEvent *ev = &e->xmaprequest;

	if(!XGetWindowAttributes(dpy, ev->window, &wa))
		return;
	if(wa.override_redirect)
		return;
	if(!getclient(ev->window))
		manage(ev->window, &wa);
}

void
monocle(void) {
	Client *c;

	for(c = nexttiled(clients); c; c = nexttiled(c->next))
		resize(c, wx, wy, ww - 2 * c->bw, wh - 2 * c->bw, resizehints);
}

void
movemouse(const Arg *arg) {
	int x, y, ocx, ocy, di, nx, ny;
	unsigned int dui;
	Client *c;
	Window dummy;
	XEvent ev;

	if(!(c = sel))
		return;
	restack();
	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
	None, cursor[CurMove], CurrentTime) != GrabSuccess)
		return;
	XQueryPointer(dpy, root, &dummy, &dummy, &x, &y, &di, &di, &dui);
	if(usegrab)
		XGrabServer(dpy);
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);
			if(snap && nx >= wx && nx <= wx + ww
			        && ny >= wy && ny <= wy + wh) {
				if(abs(wx - nx) < snap)
					nx = wx;
				else if(abs((wx + ww) - (nx + WIDTH(c))) < snap)
					nx = wx + ww - WIDTH(c);
				if(abs(wy - ny) < snap)
					ny = wy;
				else if(abs((wy + wh) - (ny + HEIGHT(c))) < snap)
					ny = wy + wh - HEIGHT(c);
				if(!c->isfloating && lt[sellt]->arrange && (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
					togglefloating(NULL);
			}
			if(!lt[sellt]->arrange || c->isfloating)
				resize(c, nx, ny, c->w, c->h, False);
			break;
		}
	}
	while(ev.type != ButtonRelease);
	if(usegrab)
		XUngrabServer(dpy);
	XUngrabPointer(dpy, CurrentTime);
}

Client *
nexttiled(Client *c) {
	for(; c && (c->isfloating || !ISVISIBLE(c)); c = c->next);
	return c;
}

void
propertynotify(XEvent *e) {
	Client *c;
	Window trans;
	XPropertyEvent *ev = &e->xproperty;

	if(ev->state == PropertyDelete)
		return; /* ignore */
	if((c = getclient(ev->window))) {
		switch (ev->atom) {
		default: break;
		case XA_WM_TRANSIENT_FOR:
			XGetTransientForHint(dpy, c->win, &trans);
			if(!c->isfloating && (c->isfloating = (getclient(trans) != NULL)))
				arrange();
			break;
		case XA_WM_NORMAL_HINTS:
			updatesizehints(c);
			break;
		case XA_WM_HINTS:
			updatewmhints(c);
			drawbar();
			break;
		}
		if(ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
			updatetitle(c);
			if(c == sel)
				drawbar();
		}
	}
}

void
quit(const Arg *arg) {
	readin = running = False;
}

void
resize(Client *c, int x, int y, int w, int h, Bool sizehints) {
	XWindowChanges wc;

	if(sizehints) {
		/* see last two sentences in ICCCM 4.1.2.3 */
		Bool baseismin = c->basew == c->minw && c->baseh == c->minh;

		/* set minimum possible */
		w = MAX(1, w);
		h = MAX(1, h);

		if(!baseismin) { /* temporarily remove base dimensions */
			w -= c->basew;
			h -= c->baseh;
		}

		/* adjust for aspect limits */
		if(c->mina > 0 && c->maxa > 0) {
			if(c->maxa < (float)w / h)
				w = h * c->maxa;
			else if(c->mina < (float)h / w)
				h = w * c->mina;
		}

		if(baseismin) { /* increment calculation requires this */
			w -= c->basew;
			h -= c->baseh;
		}

		/* adjust for increment value */
		if(c->incw)
			w -= w % c->incw;
		if(c->inch)
			h -= h % c->inch;

		/* restore base dimensions */
		w += c->basew;
		h += c->baseh;

		w = MAX(w, c->minw);
		h = MAX(h, c->minh);

		if(c->maxw)
			w = MIN(w, c->maxw);

		if(c->maxh)
			h = MIN(h, c->maxh);
	}
	if(w <= 0 || h <= 0)
		return;
	if(x > sx + sw)
		x = sw - WIDTH(c);
	if(y > sy + sh)
		y = sh - HEIGHT(c);
	if(x + w + 2 * c->bw < sx)
		x = sx;
	if(y + h + 2 * c->bw < sy)
		y = sy;
	if(h < bh)
		h = bh;
	if(w < bh)
		w = bh;
	if(c->x != x || c->y != y || c->w != w || c->h != h) {
		c->x = wc.x = x;
		c->y = wc.y = y;
		c->w = wc.width = w;
		c->h = wc.height = h;
		wc.border_width = c->bw;
		XConfigureWindow(dpy, c->win,
				CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
		configure(c);
		XSync(dpy, False);
	}
}

void
resizemouse(const Arg *arg) {
	int ocx, ocy;
	int nw, nh;
	Client *c;
	XEvent ev;

	if(!(c = sel))
		return;
	restack();
	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
	None, cursor[CurResize], CurrentTime) != GrabSuccess)
		return;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	if(usegrab)
		XGrabServer(dpy);
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			nw = MAX(ev.xmotion.x - ocx - 2*c->bw + 1, 1);
			nh = MAX(ev.xmotion.y - ocy - 2*c->bw + 1, 1);

			if(snap && nw >= wx && nw <= wx + ww
			        && nh >= wy && nh <= wy + wh) {
				if(!c->isfloating && lt[sellt]->arrange
				   && (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
					togglefloating(NULL);
			}
			if(!lt[sellt]->arrange || c->isfloating)
				resize(c, c->x, c->y, nw, nh, True);
			break;
		}
	}
	while(ev.type != ButtonRelease);
	if(usegrab)
		XUngrabServer(dpy);
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	XUngrabPointer(dpy, CurrentTime);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
restack(void) {
	Client *c;
	XEvent ev;
	XWindowChanges wc;

	drawbar();
	if(!sel)
		return;
	if(sel->isfloating || !lt[sellt]->arrange)
		XRaiseWindow(dpy, sel->win);
	if(lt[sellt]->arrange) {
		wc.stack_mode = Below;
		wc.sibling = barwin;
		for(c = stack; c; c = c->snext)
			if(!c->isfloating && ISVISIBLE(c)) {
				XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
				wc.sibling = c->win;
			}
	}
	XSync(dpy, False);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
run(void) {
	char *p;
	char sbuf[sizeof stext];
	fd_set rd;
	int r, xfd;
	unsigned int len, offset;
	XEvent ev;

	/* main event loop, also reads status text from stdin */
	XSync(dpy, False);
	xfd = ConnectionNumber(dpy);
	offset = 0;
	len = sizeof stext - 1;
	sbuf[len] = stext[len] = '\0'; /* 0-terminator is never touched */
	while(running) {
		FD_ZERO(&rd);
		if(readin)
			FD_SET(STDIN_FILENO, &rd);
		FD_SET(xfd, &rd);
		if(select(xfd + 1, &rd, NULL, NULL, NULL) == -1) {
			if(errno == EINTR)
				continue;
			die("select failed\n");
		}
		if(FD_ISSET(STDIN_FILENO, &rd)) {
			switch((r = read(STDIN_FILENO, sbuf + offset, len - offset))) {
			case -1:
				strncpy(stext, strerror(errno), len);
				readin = False;
				break;
			case 0:
				strncpy(stext, "EOF", 4);
				readin = False;
				break;
			default:
				for(p = sbuf + offset; r > 0; p++, r--, offset++)
					if(*p == '\n' || *p == '\0') {
						*p = '\0';
						strncpy(stext, sbuf, len);
						p += r - 1; /* p is sbuf + offset + r - 1 */
						for(r = 0; *(p - r) && *(p - r) != '\n'; r++);
						offset = r;
						if(r)
							memmove(sbuf, p - r + 1, r);
						break;
					}
				break;
			}
			drawbar();
		}
		while(XPending(dpy)) {
			XNextEvent(dpy, &ev);
			if(handler[ev.type])
				(handler[ev.type])(&ev); /* call handler */
		}
	}
}

void
scan(void) {
	unsigned int i, num;
	Window d1, d2, *wins = NULL;
	XWindowAttributes wa;

	if(XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for(i = 0; i < num; i++) {
			if(!XGetWindowAttributes(dpy, wins[i], &wa)
			|| wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if(wa.map_state == IsViewable || getstate(wins[i]) == IconicState)
				manage(wins[i], &wa);
		}
		for(i = 0; i < num; i++) { /* now the transients */
			if(!XGetWindowAttributes(dpy, wins[i], &wa))
				continue;
			if(XGetTransientForHint(dpy, wins[i], &d1)
			&& (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
				manage(wins[i], &wa);
		}
		if(wins)
			XFree(wins);
	}
}

void
setclientstate(Client *c, long state) {
	long data[] = {state, None};

	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
			PropModeReplace, (unsigned char *)data, 2);
}

void
setlayout(const Arg *arg) {
	if(!arg || !arg->v || arg->v != lt[sellt])
		sellt ^= 1;
	if(arg && arg->v)
		lt[sellt] = (Layout *)arg->v;
	if(sel)
		arrange();
	else
		drawbar();
}

/* arg > 1.0 will set mfact absolutly */
void
setmfact(const Arg *arg) {
	float f;

	if(!arg || !lt[sellt]->arrange)
		return;
	f = arg->f < 1.0 ? arg->f + mfact : arg->f - 1.0;
	if(f < 0.1 || f > 0.9)
		return;
	mfact = f;
	arrange();
}

void
setup(void) {
	unsigned int i;
	int w;
	XSetWindowAttributes wa;

	/* init screen */
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	initfont(font);
	sx = 0;
	sy = 0;
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	bh = dc.h = dc.font.height + 2;
	lt[0] = &layouts[0];
	lt[1] = &layouts[1 % LENGTH(layouts)];
	updategeom();

	/* init atoms */
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
	netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);

	/* init cursors */
	wa.cursor = cursor[CurNormal] = XCreateFontCursor(dpy, XC_left_ptr);
	cursor[CurResize] = XCreateFontCursor(dpy, XC_sizing);
	cursor[CurMove] = XCreateFontCursor(dpy, XC_fleur);

	/* init appearance */
	dc.norm[ColBorder] = getcolor(normbordercolor);
	dc.norm[ColBG] = getcolor(normbgcolor);
	dc.norm[ColFG] = getcolor(normfgcolor);
	dc.sel[ColBorder] = getcolor(selbordercolor);
	dc.sel[ColBG] = getcolor(selbgcolor);
	dc.sel[ColFG] = getcolor(selfgcolor);
	dc.drawable = XCreatePixmap(dpy, root, DisplayWidth(dpy, screen), bh, DefaultDepth(dpy, screen));
	dc.gc = XCreateGC(dpy, root, 0, 0);
	XSetLineAttributes(dpy, dc.gc, 1, LineSolid, CapButt, JoinMiter);
	if(!dc.font.set)
		XSetFont(dpy, dc.gc, dc.font.xfont->fid);

	/* init bar */
	for(blw = i = 0; LENGTH(layouts) > 1 && i < LENGTH(layouts); i++) {
		w = TEXTW(layouts[i].symbol);
		blw = MAX(blw, w);
	}

	wa.override_redirect = 1;
	wa.background_pixmap = ParentRelative;
	wa.event_mask = ButtonPressMask|ExposureMask;

	barwin = XCreateWindow(dpy, root, wx, by, ww, bh, 0, DefaultDepth(dpy, screen),
			CopyFromParent, DefaultVisual(dpy, screen),
			CWOverrideRedirect|CWBackPixmap|CWEventMask, &wa);
	XDefineCursor(dpy, barwin, cursor[CurNormal]);
	XMapRaised(dpy, barwin);
	strcpy(stext, "dwm-"VERSION);
	drawbar();

	/* EWMH support per view */
	XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
			PropModeReplace, (unsigned char *) netatom, NetLast);

	/* select for events */
	wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask|ButtonPressMask
			|EnterWindowMask|LeaveWindowMask|StructureNotifyMask;
	XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
	XSelectInput(dpy, root, wa.event_mask);

	grabkeys();
}

void
showhide(Client *c) {
	if(!c)
		return;
	if(ISVISIBLE(c)) { /* show clients top down */
		XMoveWindow(dpy, c->win, c->x, c->y);
		if(!lt[sellt]->arrange || c->isfloating)
			resize(c, c->x, c->y, c->w, c->h, True);
		showhide(c->snext);
	}
	else { /* hide clients bottom up */
		showhide(c->snext);
		XMoveWindow(dpy, c->win, c->x + 2 * sw, c->y);
	}
}


void
sigchld(int signal) {
	while(0 < waitpid(-1, NULL, WNOHANG));
}

void
spawn(const Arg *arg) {
	signal(SIGCHLD, sigchld);
	if(fork() == 0) {
		if(dpy)
			close(ConnectionNumber(dpy));
		setsid();
		execvp(((char **)arg->v)[0], (char **)arg->v);
		fprintf(stderr, "dwm: execvp %s", ((char **)arg->v)[0]);
		perror(" failed");
		exit(0);
	}
}

void
tag(const Arg *arg) {
	if(sel && arg->ui & TAGMASK) {
		sel->tags = arg->ui & TAGMASK;
		arrange();
	}
}

int
textnw(const char *text, unsigned int len) {
	XRectangle r;

	if(dc.font.set) {
		XmbTextExtents(dc.font.set, text, len, NULL, &r);
		return r.width;
	}
	return XTextWidth(dc.font.xfont, text, len);
}

void
tile(void) {
	int x, y, h, w, mw;
	unsigned int i, n;
	Client *c;

	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next), n++);
	if(n == 0)
		return;

	/* master */
	c = nexttiled(clients);
	mw = mfact * ww;
	resize(c, wx, wy, (n == 1 ? ww : mw) - 2 * c->bw, wh - 2 * c->bw, resizehints);

	if(--n == 0)
		return;

	/* tile stack */
	x = (wx + mw > c->x + c->w) ? c->x + c->w + 2 * c->bw : wx + mw;
	y = wy;
	w = (wx + mw > c->x + c->w) ? wx + ww - x : ww - mw;
	h = wh / n;
	if(h < bh)
		h = wh;

	for(i = 0, c = nexttiled(c->next); c; c = nexttiled(c->next), i++) {
		resize(c, x, y, w - 2 * c->bw, /* remainder */ ((i + 1 == n)
		       ? wy + wh - y - 2 * c->bw : h - 2 * c->bw), resizehints);
		if(h != wh)
			y = c->y + HEIGHT(c);
	}
}

void
togglebar(const Arg *arg) {
	showbar = !showbar;
	updategeom();
	updatebar();
	arrange();
}

void
togglefloating(const Arg *arg) {
	if(!sel)
		return;
	sel->isfloating = !sel->isfloating || sel->isfixed;
	if(sel->isfloating)
		resize(sel, sel->x, sel->y, sel->w, sel->h, True);
	arrange();
}

void
toggletag(const Arg *arg) {
	unsigned int mask;

	if (!sel)
		return;
	
	mask = sel->tags ^ (arg->ui & TAGMASK);
	if(sel && mask) {
		sel->tags = mask;
		arrange();
	}
}

void
toggleview(const Arg *arg) {
	unsigned int mask = tagset[seltags] ^ (arg->ui & TAGMASK);

	if(mask) {
		tagset[seltags] = mask;
		clearurgent();
		arrange();
	}
}

void
unmanage(Client *c) {
	XWindowChanges wc;

	wc.border_width = c->oldbw;
	/* The server grab construct avoids race conditions. */
	XGrabServer(dpy);
	XSetErrorHandler(xerrordummy);
	XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
	detach(c);
	detachstack(c);
	if(sel == c)
		focus(NULL);
	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
	setclientstate(c, WithdrawnState);
	free(c);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XUngrabServer(dpy);
	arrange();
}

void
unmapnotify(XEvent *e) {
	Client *c;
	XUnmapEvent *ev = &e->xunmap;

	if((c = getclient(ev->window)))
		unmanage(c);
}

void
updatebar(void) {
	if(dc.drawable != 0)
		XFreePixmap(dpy, dc.drawable);
	dc.drawable = XCreatePixmap(dpy, root, ww, bh, DefaultDepth(dpy, screen));
	XMoveResizeWindow(dpy, barwin, wx, by, ww, bh);
}

void
updategeom(void) {
#ifdef XINERAMA
	int n, i = 0;
	XineramaScreenInfo *info = NULL;

	/* window area geometry */
	if(XineramaIsActive(dpy) && (info = XineramaQueryScreens(dpy, &n))) { 
		if(n > 1) {
			int di, x, y;
			unsigned int dui;
			Window dummy;
			if(XQueryPointer(dpy, root, &dummy, &dummy, &x, &y, &di, &di, &dui))
				for(i = 0; i < n; i++)
					if(INRECT(x, y, info[i].x_org, info[i].y_org, info[i].width, info[i].height))
						break;
		}
		wx = info[i].x_org;
		wy = showbar && topbar ?  info[i].y_org + bh : info[i].y_org;
		ww = info[i].width;
		wh = showbar ? info[i].height - bh : info[i].height;
		XFree(info);
	}
	else
#endif
	{
		wx = sx;
		wy = showbar && topbar ? sy + bh : sy;
		ww = sw;
		wh = showbar ? sh - bh : sh;
	}

	/* bar position */
	by = showbar ? (topbar ? wy - bh : wy + wh) : -bh;
}

void
updatenumlockmask(void) {
	unsigned int i, j;
	XModifierKeymap *modmap;

	numlockmask = 0;
	modmap = XGetModifierMapping(dpy);
	for(i = 0; i < 8; i++)
		for(j = 0; j < modmap->max_keypermod; j++)
			if(modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
	XFreeModifiermap(modmap);
}

void
updatesizehints(Client *c) {
	long msize;
	XSizeHints size;

	if(!XGetWMNormalHints(dpy, c->win, &size, &msize))
		/* size is uninitialized, ensure that size.flags aren't used */
		size.flags = PSize; 
	if(size.flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	}
	else if(size.flags & PMinSize) {
		c->basew = size.min_width;
		c->baseh = size.min_height;
	}
	else
		c->basew = c->baseh = 0;
	if(size.flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	}
	else
		c->incw = c->inch = 0;
	if(size.flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	}
	else
		c->maxw = c->maxh = 0;
	if(size.flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	}
	else if(size.flags & PBaseSize) {
		c->minw = size.base_width;
		c->minh = size.base_height;
	}
	else
		c->minw = c->minh = 0;
	if(size.flags & PAspect) {
		c->mina = (float)size.min_aspect.y / (float)size.min_aspect.x;
		c->maxa = (float)size.max_aspect.x / (float)size.max_aspect.y;
	}
	else
		c->maxa = c->mina = 0.0;
	c->isfixed = (c->maxw && c->minw && c->maxh && c->minh
	             && c->maxw == c->minw && c->maxh == c->minh);
}

void
updatetitle(Client *c) {
	if(!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
		gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
}

void
updatewmhints(Client *c) {
	XWMHints *wmh;

	if((wmh = XGetWMHints(dpy, c->win))) {
		if(ISVISIBLE(c) && wmh->flags & XUrgencyHint) {
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dpy, c->win, wmh);
		}
		else
			c->isurgent = (wmh->flags & XUrgencyHint) ? True : False;

		XFree(wmh);
	}
}

void
view(const Arg *arg) {
	if((arg->ui & TAGMASK) == tagset[seltags])
		return;
	seltags ^= 1; /* toggle sel tagset */
	if(arg->ui & TAGMASK)
		tagset[seltags] = arg->ui & TAGMASK;
	clearurgent();
	arrange();
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's).  Other types of errors call Xlibs
 * default error handler, which may call exit.  */
int
xerror(Display *dpy, XErrorEvent *ee) {
	if(ee->error_code == BadWindow
	|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	|| (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
	fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
			ee->request_code, ee->error_code);
	return xerrorxlib(dpy, ee); /* may call exit */
}

int
xerrordummy(Display *dpy, XErrorEvent *ee) {
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int
xerrorstart(Display *dpy, XErrorEvent *ee) {
	otherwm = True;
	return -1;
}

void
zoom(const Arg *arg) {
	Client *c = sel;

	if(!lt[sellt]->arrange || lt[sellt]->arrange == monocle || (sel && sel->isfloating))
		return;
	if(c == nexttiled(clients))
		if(!c || !(c = nexttiled(c->next)))
			return;
	detach(c);
	attach(c);
	focus(c);
	arrange();
}

int
main(int argc, char *argv[]) {
	if(argc == 2 && !strcmp("-v", argv[1]))
		die("dwm-"VERSION", © 2006-2008 dwm engineers, see LICENSE for details\n");
	else if(argc != 1)
		die("usage: dwm [-v]\n");

	if(!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fprintf(stderr, "warning: no locale support\n");

	if(!(dpy = XOpenDisplay(0)))
		die("dwm: cannot open display\n");

	checkotherwm();
	setup();
	scan();
	run();
	cleanup();

	XCloseDisplay(dpy);
	return 0;
}
