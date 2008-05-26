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
 * doubly-linked client list, the focus history is remembered through a global
 * stack list. Each client contains an array of Bools of the same size as the
 * global tags array to indicate the tags of a client.
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
#define MAX(a, b)       ((a) > (b) ? (a) : (b))
#define MIN(a, b)       ((a) < (b) ? (a) : (b))
#define BUTTONMASK      (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask) (mask & ~(numlockmask|LockMask))
#define LENGTH(x)       (sizeof x / sizeof x[0])
#define MAXTAGLEN       16
#define MOUSEMASK       (BUTTONMASK|PointerMotionMask)
#define TAGMASK         ((int)((1LL << LENGTH(tags)) - 1))
#define TEXTW(x)        (textnw(x, strlen(x)) + dc.font.height)
#define VISIBLE(x)      ((x)->tags & tagset[seltags])

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast };        /* cursor */
enum { ColBorder, ColFG, ColBG, ColLast };              /* color */
enum { NetSupported, NetWMName, NetLast };              /* EWMH atoms */
enum { WMProtocols, WMDelete, WMName, WMState, WMLast };/* default atoms */

/* typedefs */
typedef unsigned int uint;
typedef unsigned long ulong;
typedef struct Client Client;
struct Client {
	char name[256];
	int x, y, w, h;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int minax, maxax, minay, maxay;
	long flags;
	int bw, oldbw;
	Bool isbanned, isfixed, isfloating, ismax, isurgent;
	uint tags;
	Client *next;
	Client *prev;
	Client *snext;
	Window win;
};

typedef struct {
	int x, y, w, h;
	ulong norm[ColLast];
	ulong sel[ColLast];
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
	uint mod;
	KeySym keysym;
	void (*func)(const void *arg);
	const void *arg;
} Key;

typedef struct {
	const char *symbol;
	void (*arrange)(void);
} Layout;

typedef struct {
	const char *class;
	const char *instance;
	const char *title;
	uint tags;
	Bool isfloating;
} Rule;

/* function declarations */
void applyrules(Client *c);
void arrange(void);
void attach(Client *c);
void attachstack(Client *c);
void buttonpress(XEvent *e);
void checkotherwm(void);
void cleanup(void);
void configure(Client *c);
void configurenotify(XEvent *e);
void configurerequest(XEvent *e);
void destroynotify(XEvent *e);
void detach(Client *c);
void detachstack(Client *c);
void drawbar(void);
void drawsquare(Bool filled, Bool empty, Bool invert, ulong col[ColLast]);
void drawtext(const char *text, ulong col[ColLast], Bool invert);
void enternotify(XEvent *e);
void eprint(const char *errstr, ...);
void expose(XEvent *e);
void focus(Client *c);
void focusin(XEvent *e);
void focusnext(const void *arg);
void focusprev(const void *arg);
Client *getclient(Window w);
ulong getcolor(const char *colstr);
long getstate(Window w);
Bool gettextprop(Window w, Atom atom, char *text, uint size);
void grabbuttons(Client *c, Bool focused);
void grabkeys(void);
void initfont(const char *fontstr);
Bool isoccupied(uint t);
Bool isprotodel(Client *c);
Bool isurgent(uint t);
void keypress(XEvent *e);
void killclient(const void *arg);
void manage(Window w, XWindowAttributes *wa);
void mappingnotify(XEvent *e);
void maprequest(XEvent *e);
void movemouse(Client *c);
Client *nexttiled(Client *c);
void propertynotify(XEvent *e);
void quit(const void *arg);
void resize(Client *c, int x, int y, int w, int h, Bool sizehints);
void resizemouse(Client *c);
void restack(void);
void run(void);
void scan(void);
void setclientstate(Client *c, long state);
void setmfact(const void *arg);
void setup(void);
void spawn(const void *arg);
void tag(const void *arg);
uint textnw(const char *text, uint len);
void tile(void);
void togglebar(const void *arg);
void togglefloating(const void *arg);
void togglelayout(const void *arg);
void togglemax(const void *arg);
void toggletag(const void *arg);
void toggleview(const void *arg);
void unmanage(Client *c);
void unmapnotify(XEvent *e);
void updatebar(void);
void updategeom(void);
void updatesizehints(Client *c);
void updatetitle(Client *c);
void updatewmhints(Client *c);
void view(const void *arg);
void viewprevtag(const void *arg);
int xerror(Display *dpy, XErrorEvent *ee);
int xerrordummy(Display *dpy, XErrorEvent *ee);
int xerrorstart(Display *dpy, XErrorEvent *ee);
void zoom(const void *arg);

/* variables */
char stext[256];
int screen, sx, sy, sw, sh;
int bx, by, bw, bh, blw, wx, wy, ww, wh;
uint seltags = 0;
int (*xerrorxlib)(Display *, XErrorEvent *);
uint numlockmask = 0;
void (*handler[LASTEvent]) (XEvent *) = {
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
Atom wmatom[WMLast], netatom[NetLast];
Bool ismax = False;
Bool otherwm, readin;
Bool running = True;
uint tagset[] = {1, 1}; /* after start, first tag is selected */
Client *clients = NULL;
Client *sel = NULL;
Client *stack = NULL;
Cursor cursor[CurLast];
Display *dpy;
DC dc = {0};
Layout layouts[];
Layout *lt = layouts;
Window root, barwin;

/* configuration, allows nested code to access above variables */
#include "config.h"

/* compile-time check if all tags fit into an uint bit array. */
struct NumTags { char limitexceeded[sizeof(uint) * 8 < LENGTH(tags) ? -1 : 1]; };

/* function implementations */
void
applyrules(Client *c) {
	uint i;
	Rule *r;
	XClassHint ch = { 0 };

	/* rule matching */
	XGetClassHint(dpy, c->win, &ch);
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
	if(!c->tags)
		c->tags = tagset[seltags];
}

void
arrange(void) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(VISIBLE(c)) {
			if(!lt->arrange || c->isfloating)
				resize(c, c->x, c->y, c->w, c->h, True);
		}
		else if(!c->isbanned) {
			XMoveWindow(dpy, c->win, c->x + 2 * sw, c->y);
			c->isbanned = True;
		}

	focus(NULL);
	if(lt->arrange && !ismax)
		lt->arrange();
	restack();
}

void
attach(Client *c) {
	if(clients)
		clients->prev = c;
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
	uint i, x, mask;
	Client *c;
	XButtonPressedEvent *ev = &e->xbutton;

	if(ev->window == barwin) {
		x = 0;
		for(i = 0; i < LENGTH(tags); i++) {
			x += TEXTW(tags[i]);
			if(ev->x < x) {
				mask = 1 << i;
				if(ev->button == Button1) {
					if(ev->state & MODKEY)
						tag(&mask);
					else
						view(&mask);
				}
				else if(ev->button == Button3) {
					if(ev->state & MODKEY)
						toggletag(&mask);
					else
						toggleview(&mask);
				}
				return;
			}
		}
		if(ev->x < x + blw) {
			if(ev->button == Button1) 
				togglelayout(NULL);
			else if(ev->button == Button3) 
				togglemax(NULL);
		}
	}
	else if((c = getclient(ev->window))) {
		focus(c);
		if(CLEANMASK(ev->state) != MODKEY || ismax)
			return;
		if(ev->button == Button1)
			movemouse(c);
		else if(ev->button == Button2)
			togglefloating(NULL);
		else if(ev->button == Button3 && !c->isfixed)
			resizemouse(c);
	}
}

void
checkotherwm(void) {
	otherwm = False;
	XSetErrorHandler(xerrorstart);

	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
	XSync(dpy, False);
	if(otherwm)
		eprint("dwm: another window manager is already running\n");
	XSync(dpy, False);
	XSetErrorHandler(NULL);
	xerrorxlib = XSetErrorHandler(xerror);
	XSync(dpy, False);
}

void
cleanup(void) {
	close(STDIN_FILENO);
	view(NULL);
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
		if(c->isfixed || c->isfloating || !lt->arrange) {
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
			if((ev->value_mask & (CWX|CWY))
			&& !(ev->value_mask & (CWWidth|CWHeight)))
				configure(c);
			if(VISIBLE(c))
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
	if(c->prev)
		c->prev->next = c->next;
	if(c->next)
		c->next->prev = c->prev;
	if(c == clients)
		clients = c->next;
	c->next = c->prev = NULL;
}

void
detachstack(Client *c) {
	Client **tc;

	for(tc = &stack; *tc && *tc != c; tc = &(*tc)->snext);
	*tc = c->snext;
}

void
drawbar(void) {
	int i, x;
	Client *c;

	dc.x = 0;
	for(c = stack; c && !VISIBLE(c); c = c->snext);
	for(i = 0; i < LENGTH(tags); i++) {
		dc.w = TEXTW(tags[i]);
		if(tagset[seltags] & 1 << i) {
			drawtext(tags[i], dc.sel, isurgent(i));
			drawsquare(c && c->tags & 1 << i, isoccupied(i), isurgent(i), dc.sel);
		}
		else {
			drawtext(tags[i], dc.norm, isurgent(i));
			drawsquare(c && c->tags & 1 << i, isoccupied(i), isurgent(i), dc.norm);
		}
		dc.x += dc.w;
	}
	if(blw > 0) {
		dc.w = blw;
		drawtext(lt->symbol, dc.norm, ismax);
		x = dc.x + dc.w;
	}
	else
		x = dc.x;
	dc.w = TEXTW(stext);
	dc.x = bw - dc.w;
	if(dc.x < x) {
		dc.x = x;
		dc.w = bw - x;
	}
	drawtext(stext, dc.norm, False);
	if((dc.w = dc.x - x) > bh) {
		dc.x = x;
		if(c) {
			drawtext(c->name, dc.sel, False);
			drawsquare(False, c->isfloating, False, dc.sel);
		}
		else
			drawtext(NULL, dc.norm, False);
	}
	XCopyArea(dpy, dc.drawable, barwin, dc.gc, 0, 0, bw, bh, 0, 0);
	XSync(dpy, False);
}

void
drawsquare(Bool filled, Bool empty, Bool invert, ulong col[ColLast]) {
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
drawtext(const char *text, ulong col[ColLast], Bool invert) {
	int x, y, w, h;
	uint len, olen;
	XRectangle r = { dc.x, dc.y, dc.w, dc.h };
	char buf[256];

	XSetForeground(dpy, dc.gc, col[invert ? ColFG : ColBG]);
	XFillRectangles(dpy, dc.drawable, dc.gc, &r, 1);
	if(!text)
		return;
	olen = strlen(text);
	len = MIN(olen, sizeof buf);
	memcpy(buf, text, len);
	w = 0;
	h = dc.font.ascent + dc.font.descent;
	y = dc.y + (dc.h / 2) - (h / 2) + dc.font.ascent;
	x = dc.x + (h / 2);
	/* shorten text if necessary */
	for(; len && (w = textnw(buf, len)) > dc.w - h; len--);
	if(!len)
		return;
	if(len < olen) {
		if(len > 1)
			buf[len - 1] = '.';
		if(len > 2)
			buf[len - 2] = '.';
		if(len > 3)
			buf[len - 3] = '.';
	}
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
eprint(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void
expose(XEvent *e) {
	XExposeEvent *ev = &e->xexpose;

	if(ev->count == 0 && (ev->window == barwin))
		drawbar();
}

void
focus(Client *c) {
	if(!c || (c && !VISIBLE(c)))
		for(c = stack; c && !VISIBLE(c); c = c->snext);
	if(sel && sel != c) {
		grabbuttons(sel, False);
		XSetWindowBorder(dpy, sel->win, dc.norm[ColBorder]);
	}
	if(c) {
		detachstack(c);
		attachstack(c);
		grabbuttons(c, True);
	}
	sel = c;
	if(c) {
		if(ismax) {
			XMoveResizeWindow(dpy, c->win, wx, wy, ww - 2 * c->bw, wh - 2 * c->bw);
			c->ismax = True;
		}
		XSetWindowBorder(dpy, c->win, dc.sel[ColBorder]);
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
	}
	else
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
	drawbar();
}

void
focusin(XEvent *e) { /* there are some broken focus acquiring clients */
	XFocusChangeEvent *ev = &e->xfocus;

	if(sel && ev->window != sel->win)
		XSetInputFocus(dpy, sel->win, RevertToPointerRoot, CurrentTime);
}

void
focusnext(const void *arg) {
	Client *c;

	if(!sel)
		return;
	for(c = sel->next; c && !VISIBLE(c); c = c->next);
	if(!c)
		for(c = clients; c && !VISIBLE(c); c = c->next);
	if(c) {
		focus(c);
		restack();
	}
}

void
focusprev(const void *arg) {
	Client *c;

	if(!sel)
		return;
	for(c = sel->prev; c && !VISIBLE(c); c = c->prev);
	if(!c) {
		for(c = clients; c && c->next; c = c->next);
		for(; c && !VISIBLE(c); c = c->prev);
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

ulong
getcolor(const char *colstr) {
	Colormap cmap = DefaultColormap(dpy, screen);
	XColor color;

	if(!XAllocNamedColor(dpy, cmap, colstr, &color, &color))
		eprint("error, cannot allocate color '%s'\n", colstr);
	return color.pixel;
}

long
getstate(Window w) {
	int format, status;
	long result = -1;
	unsigned char *p = NULL;
	ulong n, extra;
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
gettextprop(Window w, Atom atom, char *text, uint size) {
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
	int i, j;
	uint buttons[]   = { Button1, Button2, Button3 };
	uint modifiers[] = { MODKEY, MODKEY|LockMask, MODKEY|numlockmask,
				MODKEY|numlockmask|LockMask} ;

	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
	if(focused)
		for(i = 0; i < LENGTH(buttons); i++)
			for(j = 0; j < LENGTH(modifiers); j++)
				XGrabButton(dpy, buttons[i], modifiers[j], c->win, False,
					BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
	else
		XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
			BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
}

void
grabkeys(void) {
	uint i, j;
	KeyCode code;
	XModifierKeymap *modmap;

	/* init modifier map */
	modmap = XGetModifierMapping(dpy);
	for(i = 0; i < 8; i++)
		for(j = 0; j < modmap->max_keypermod; j++) {
			if(modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
		}
	XFreeModifiermap(modmap);

	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	for(i = 0; i < LENGTH(keys); i++) {
		code = XKeysymToKeycode(dpy, keys[i].keysym);
		XGrabKey(dpy, code, keys[i].mod, root, True,
				GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, code, keys[i].mod|LockMask, root, True,
				GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, code, keys[i].mod|numlockmask, root, True,
				GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, code, keys[i].mod|numlockmask|LockMask, root, True,
				GrabModeAsync, GrabModeAsync);
	}
}

void
initfont(const char *fontstr) {
	char *def, **missing;
	int i, n;

	missing = NULL;
	if(dc.font.set)
		XFreeFontSet(dpy, dc.font.set);
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
		if(dc.font.xfont)
			XFreeFont(dpy, dc.font.xfont);
		dc.font.xfont = NULL;
		if(!(dc.font.xfont = XLoadQueryFont(dpy, fontstr))
		&& !(dc.font.xfont = XLoadQueryFont(dpy, "fixed")))
			eprint("error, cannot load font: '%s'\n", fontstr);
		dc.font.ascent = dc.font.xfont->ascent;
		dc.font.descent = dc.font.xfont->descent;
	}
	dc.font.height = dc.font.ascent + dc.font.descent;
}

Bool
isoccupied(uint t) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(c->tags & 1 << t)
			return True;
	return False;
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

Bool
isurgent(uint t) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(c->isurgent && c->tags & 1 << t)
			return True;
	return False;
}

void
keypress(XEvent *e) {
	uint i;
	KeySym keysym;
	XKeyEvent *ev;

	ev = &e->xkey;
	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
	for(i = 0; i < LENGTH(keys); i++)
		if(keysym == keys[i].keysym
		&& CLEANMASK(keys[i].mod) == CLEANMASK(ev->state))
		{
			if(keys[i].func)
				keys[i].func(keys[i].arg);
		}
}

void
killclient(const void *arg) {
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
	Client *c, *t = NULL;
	Status rettrans;
	Window trans;
	XWindowChanges wc;

	if(!(c = calloc(1, sizeof(Client))))
		eprint("fatal: could not calloc() %u bytes\n", sizeof(Client));
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
		c->bw = wa->border_width;
	}
	else {
		if(c->x + c->w + 2 * c->bw > sx + sw)
			c->x = sx + sw - c->w - 2 * c->bw;
		if(c->y + c->h + 2 * c->bw > sy + sh)
			c->y = sy + sh - c->h - 2 * c->bw;
		c->x = MAX(c->x, sx);
		c->y = MAX(c->y, by == 0 ? bh : sy);
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
	if((rettrans = XGetTransientForHint(dpy, w, &trans) == Success))
		for(t = clients; t && t->win != trans; t = t->next);
	if(t)
		c->tags = t->tags;
	else
		applyrules(c);
	if(!c->isfloating)
		c->isfloating = (rettrans == Success) || c->isfixed;
	attach(c);
	attachstack(c);
	XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h); /* some windows require this */
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
movemouse(Client *c) {
	int x1, y1, ocx, ocy, di, nx, ny;
	uint dui;
	Window dummy;
	XEvent ev;

	restack();
	ocx = nx = c->x;
	ocy = ny = c->y;
	if(XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
	None, cursor[CurMove], CurrentTime) != GrabSuccess)
		return;
	XQueryPointer(dpy, root, &dummy, &dummy, &x1, &y1, &di, &di, &dui);
	for(;;) {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ButtonRelease:
			XUngrabPointer(dpy, CurrentTime);
			return;
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			XSync(dpy, False);
			nx = ocx + (ev.xmotion.x - x1);
			ny = ocy + (ev.xmotion.y - y1);
			if(snap && nx >= wx && nx <= wx + ww
			        && ny >= wy && ny <= wy + wh) {
				if(abs(wx - nx) < snap)
					nx = wx;
				else if(abs((wx + ww) - (nx + c->w + 2 * c->bw)) < snap)
					nx = wx + ww - c->w - 2 * c->bw;
				if(abs(wy - ny) < snap)
					ny = wy;
				else if(abs((wy + wh) - (ny + c->h + 2 * c->bw)) < snap)
					ny = wy + wh - c->h - 2 * c->bw;
				if(!c->isfloating && lt->arrange && (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
					togglefloating(NULL);
			}
			if(!lt->arrange || c->isfloating)
				resize(c, nx, ny, c->w, c->h, False);
			break;
		}
	}
}

Client *
nexttiled(Client *c) {
	for(; c && (c->isfloating || !VISIBLE(c)); c = c->next);
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
quit(const void *arg) {
	readin = running = False;
}

void
resize(Client *c, int x, int y, int w, int h, Bool sizehints) {
	XWindowChanges wc;

	if(sizehints) {
		/* set minimum possible */
		w = MAX(1, w);
		h = MAX(1, h);

		/* temporarily remove base dimensions */
		w -= c->basew;
		h -= c->baseh;

		/* adjust for aspect limits */
		if(c->minax != c->maxax && c->minay != c->maxay 
		&& c->minax > 0 && c->maxax > 0 && c->minay > 0 && c->maxay > 0) {
			if(w * c->maxay > h * c->maxax)
				w = h * c->maxax / c->maxay;
			else if(w * c->minay < h * c->minax)
				h = w * c->minay / c->minax;
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
		
		if (c->maxw)
			w = MIN(w, c->maxw);

		if (c->maxh)
			h = MIN(h, c->maxh);
	}
	if(w <= 0 || h <= 0)
		return;
	if(x > sx + sw)
		x = sw - w - 2 * c->bw;
	if(y > sy + sh)
		y = sh - h - 2 * c->bw;
	if(x + w + 2 * c->bw < sx)
		x = sx;
	if(y + h + 2 * c->bw < sy)
		y = sy;
	if(h < bh)
		h = bh;
	if(w < bh)
		w = bh;
	if(c->x != x || c->y != y || c->w != w || c->h != h || c->isbanned || c->ismax) {
		c->isbanned = c->ismax = False;
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
resizemouse(Client *c) {
	int ocx, ocy;
	int nw, nh;
	XEvent ev;

	restack();
	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
	None, cursor[CurResize], CurrentTime) != GrabSuccess)
		return;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	for(;;) {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask , &ev);
		switch(ev.type) {
		case ButtonRelease:
			XWarpPointer(dpy, None, c->win, 0, 0, 0, 0,
					c->w + c->bw - 1, c->h + c->bw - 1);
			XUngrabPointer(dpy, CurrentTime);
			while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
			return;
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			XSync(dpy, False);
			nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
			nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);

			if(snap && nw >= wx && nw <= wx + ww
			        && nh >= wy && nh <= wy + wh) {
				if(!c->isfloating && lt->arrange
				   && (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
					togglefloating(NULL);
			}
			if(!lt->arrange || c->isfloating)
				resize(c, c->x, c->y, nw, nh, True);
			break;
		}
	}
}

void
restack(void) {
	Client *c;
	XEvent ev;
	XWindowChanges wc;

	drawbar();
	if(!sel)
		return;
	if(ismax || sel->isfloating || !lt->arrange)
		XRaiseWindow(dpy, sel->win);
	if(!ismax && lt->arrange) {
		wc.stack_mode = Below;
		wc.sibling = barwin;
		for(c = stack; c; c = c->snext)
			if(!c->isfloating && VISIBLE(c)) {
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
	uint len, offset;
	XEvent ev;

	/* main event loop, also reads status text from stdin */
	XSync(dpy, False);
	xfd = ConnectionNumber(dpy);
	readin = True;
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
			eprint("select failed\n");
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
	uint i, num;
	Window *wins, d1, d2;
	XWindowAttributes wa;

	wins = NULL;
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
	}
	if(wins)
		XFree(wins);
}

void
setclientstate(Client *c, long state) {
	long data[] = {state, None};

	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
			PropModeReplace, (unsigned char *)data, 2);
}

/* arg > 1.0 will set mfact absolutly */
void
setmfact(const void *arg) {
	double d = *((double*) arg);

	if(!d || lt->arrange != tile)
		return;
	d = d < 1.0 ? d + mfact : d - 1.0;
	if(d < 0.1 || d > 0.9)
		return;
	mfact = d;
	arrange();
}

void
setup(void) {
	uint i, w;
	XSetWindowAttributes wa;

	/* init screen */
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	initfont(FONT);
	sx = 0;
	sy = 0;
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	bh = dc.font.height + 2;
	updategeom();

	/* init atoms */
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMName] = XInternAtom(dpy, "WM_NAME", False);
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
	netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);

	/* init cursors */
	wa.cursor = cursor[CurNormal] = XCreateFontCursor(dpy, XC_left_ptr);
	cursor[CurResize] = XCreateFontCursor(dpy, XC_sizing);
	cursor[CurMove] = XCreateFontCursor(dpy, XC_fleur);

	/* init appearance */
	dc.norm[ColBorder] = getcolor(NORMBORDERCOLOR);
	dc.norm[ColBG] = getcolor(NORMBGCOLOR);
	dc.norm[ColFG] = getcolor(NORMFGCOLOR);
	dc.sel[ColBorder] = getcolor(SELBORDERCOLOR);
	dc.sel[ColBG] = getcolor(SELBGCOLOR);
	dc.sel[ColFG] = getcolor(SELFGCOLOR);
	initfont(FONT);
	dc.h = bh;
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

	barwin = XCreateWindow(dpy, root, bx, by, bw, bh, 0, DefaultDepth(dpy, screen),
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
	wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask
			|EnterWindowMask|LeaveWindowMask|StructureNotifyMask;
	XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
	XSelectInput(dpy, root, wa.event_mask);


	/* grab keys */
	grabkeys();
}

void
spawn(const void *arg) {
	static char *shell = NULL;

	if(!shell && !(shell = getenv("SHELL")))
		shell = "/bin/sh";
	/* The double-fork construct avoids zombie processes and keeps the code
	 * clean from stupid signal handlers. */
	if(fork() == 0) {
		if(fork() == 0) {
			if(dpy)
				close(ConnectionNumber(dpy));
			setsid();
			execl(shell, shell, "-c", (char *)arg, (char *)NULL);
			fprintf(stderr, "dwm: execl '%s -c %s'", shell, (char *)arg);
			perror(" failed");
		}
		exit(0);
	}
	wait(0);
}

void
tag(const void *arg) {
	if(sel && *(int *)arg & TAGMASK) {
		sel->tags = *(int *)arg & TAGMASK;
		arrange();
	}
}

uint
textnw(const char *text, uint len) {
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
	uint i, n;
	Client *c;

	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next), n++);
	if(n == 0)
		return;

	/* master */
	c = nexttiled(clients);
	mw = mfact * ww;
	resize(c, wx, wy, ((n == 1) ? ww : mw) - 2 * c->bw, wh - 2 * c->bw, resizehints);

	if(--n == 0)
		return;

	/* tile stack */
	x = (wx + mw > c->x + c->w) ? c->x + c->w + 2 * c->bw : ww - mw;
	y = wy;
	w = (wx + mw > c->x + c->w) ? wx + ww - x : ww - mw;
	h = wh / n;
	if(h < bh)
		h = wh;

	for(i = 0, c = nexttiled(c->next); c; c = nexttiled(c->next), i++) {
		resize(c, x, y, w - 2 * c->bw, /* remainder */ ((i + 1 == n)
		       ? (wy + wh) - y : h) - 2 * c->bw, resizehints);
		if(h != wh)
			y = c->y + c->h + 2 * c->bw;
	}
}

void
togglebar(const void *arg) {
	showbar = !showbar;
	updategeom();
	updatebar();
	arrange();
}

void
togglefloating(const void *arg) {
	if(!sel)
		return;
	sel->isfloating = !sel->isfloating;
	if(sel->isfloating)
		resize(sel, sel->x, sel->y, sel->w, sel->h, True);
	arrange();
}

void
togglelayout(const void *arg) {
	uint i;

	if(!arg) {
		if(++lt == &layouts[LENGTH(layouts)])
			lt = &layouts[0];
	}
	else {
		for(i = 0; i < LENGTH(layouts); i++)
			if(!strcmp((char *)arg, layouts[i].symbol))
				break;
		if(i == LENGTH(layouts))
			return;
		lt = &layouts[i];
	}
	if(sel)
		arrange();
	else
		drawbar();
}

void
togglemax(const void *arg) {
	ismax = !ismax;
	arrange();
}

void
toggletag(const void *arg) {
	if(sel && (sel->tags ^ ((*(int *)arg) & TAGMASK))) {
		sel->tags ^= (*(int *)arg) & TAGMASK;
		arrange();
	}
}

void
toggleview(const void *arg) {
	if((tagset[seltags] ^ ((*(int *)arg) & TAGMASK))) {
		tagset[seltags] ^= (*(int *)arg) & TAGMASK;
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
	dc.drawable = XCreatePixmap(dpy, root, bw, bh, DefaultDepth(dpy, screen));
	XMoveResizeWindow(dpy, barwin, bx, by, bw, bh);
}

void
updategeom(void) {
	int i;
#ifdef XINERAMA
	XineramaScreenInfo *info = NULL;

	/* window area geometry */
	if(XineramaIsActive(dpy)) {
		info = XineramaQueryScreens(dpy, &i);
		wx = info[0].x_org;
		wy = showbar && topbar ? info[0].y_org + bh : info[0].y_org;
		ww = info[0].width;
		wh = showbar ? info[0].height - bh : info[0].height;
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

	/* bar geometry */
	bx = wx;
	by = showbar ? (topbar ? wy - bh : wy + wh) : -bh;
	bw = ww;
}

void
updatesizehints(Client *c) {
	long msize;
	XSizeHints size;

	if(!XGetWMNormalHints(dpy, c->win, &size, &msize) || !size.flags)
		size.flags = PSize;
	c->flags = size.flags;
	if(c->flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	}
	else if(c->flags & PMinSize) {
		c->basew = size.min_width;
		c->baseh = size.min_height;
	}
	else
		c->basew = c->baseh = 0;
	if(c->flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	}
	else
		c->incw = c->inch = 0;
	if(c->flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	}
	else
		c->maxw = c->maxh = 0;
	if(c->flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	}
	else if(c->flags & PBaseSize) {
		c->minw = size.base_width;
		c->minh = size.base_height;
	}
	else
		c->minw = c->minh = 0;
	if(c->flags & PAspect) {
		c->minax = size.min_aspect.x;
		c->maxax = size.max_aspect.x;
		c->minay = size.min_aspect.y;
		c->maxay = size.max_aspect.y;
	}
	else
		c->minax = c->maxax = c->minay = c->maxay = 0;
	c->isfixed = (c->maxw && c->minw && c->maxh && c->minh
			&& c->maxw == c->minw && c->maxh == c->minh);
}

void
updatetitle(Client *c) {
	if(!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
		gettextprop(c->win, wmatom[WMName], c->name, sizeof c->name);
}

void
updatewmhints(Client *c) {
	XWMHints *wmh;

	if((wmh = XGetWMHints(dpy, c->win))) {
		if(c == sel)
			sel->isurgent = False;
		else
			c->isurgent = (wmh->flags & XUrgencyHint) ? True : False;
		XFree(wmh);
	}
}

void
view(const void *arg) {
	if(*(int *)arg & TAGMASK) {
		seltags ^= 1; /* toggle sel tagset */
		tagset[seltags] = *(int *)arg & TAGMASK;
		arrange();
	}
}

void
viewprevtag(const void *arg) {
	seltags ^= 1; /* toggle sel tagset */
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
zoom(const void *arg) {
	Client *c = sel;

	if(!lt->arrange || sel->isfloating)
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
		eprint("dwm-"VERSION", © 2006-2008 dwm engineers, see LICENSE for details\n");
	else if(argc != 1)
		eprint("usage: dwm [-v]\n");

	setlocale(LC_CTYPE, "");
	if(!(dpy = XOpenDisplay(0)))
		eprint("dwm: cannot open display\n");

	checkotherwm();
	setup();
	scan();
	run();
	cleanup();

	XCloseDisplay(dpy);
	return 0;
}
