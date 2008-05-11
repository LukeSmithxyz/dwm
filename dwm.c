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

/* macros */
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define BUTTONMASK	(ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)	(mask & ~(numlockmask|LockMask))
#define LENGTH(x)	(sizeof x / sizeof x[0])
#define MAXTAGLEN	16
#define MOUSEMASK	(BUTTONMASK|PointerMotionMask)

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast };	/* cursor */
enum { ColBorder, ColFG, ColBG, ColLast };		/* color */
enum { NetSupported, NetWMName, NetLast };		/* EWMH atoms */
enum { WMProtocols, WMDelete, WMName, WMState, WMLast };/* default atoms */

/* typedefs */
typedef struct Client Client;
struct Client {
	char name[256];
	int x, y, w, h;
	int fx, fy, fw, fh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int minax, maxax, minay, maxay;
	long flags;
	unsigned int bw, oldbw;
	Bool isbanned, isfixed, isfloating, isurgent;
	Bool *tags;
	Client *next;
	Client *prev;
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
	unsigned long mod;
	KeySym keysym;
	void (*func)(const char *arg);
	const char *arg;
} Key;

typedef struct {
	const char *symbol;
	void (*arrange)(void);
	Bool isfloating;
} Layout;

typedef struct {
	const char *class;
	const char *instance;
	const char *title;
	const char *tag;
	Bool isfloating;
} Rule;

/* function declarations */
void applyrules(Client *c);
void arrange(void);
void attach(Client *c);
void attachstack(Client *c);
void ban(Client *c);
void buttonpress(XEvent *e);
void checkotherwm(void);
void cleanup(void);
void configure(Client *c);
void configurenotify(XEvent *e);
void configurerequest(XEvent *e);
unsigned int counttiled(void);
void destroynotify(XEvent *e);
void detach(Client *c);
void detachstack(Client *c);
void drawbar(void);
void drawsquare(Bool filled, Bool empty, Bool invert, unsigned long col[ColLast]);
void drawtext(const char *text, unsigned long col[ColLast], Bool invert);
void *emallocz(unsigned int size);
void enternotify(XEvent *e);
void eprint(const char *errstr, ...);
void expose(XEvent *e);
void focus(Client *c);
void focusin(XEvent *e);
void focusnext(const char *arg);
void focusprev(const char *arg);
Client *getclient(Window w);
unsigned long getcolor(const char *colstr);
long getstate(Window w);
Bool gettextprop(Window w, Atom atom, char *text, unsigned int size);
void grabbuttons(Client *c, Bool focused);
void grabkeys(void);
unsigned int idxoftag(const char *t);
void initfont(const char *fontstr);
Bool isoccupied(unsigned int t);
Bool isprotodel(Client *c);
Bool isurgent(unsigned int t);
Bool isvisible(Client *c, Bool *cmp);
void keypress(XEvent *e);
void killclient(const char *arg);
void manage(Window w, XWindowAttributes *wa);
void mappingnotify(XEvent *e);
void maprequest(XEvent *e);
void monocle(void);
void movemouse(Client *c);
Client *nexttiled(Client *c);
void propertynotify(XEvent *e);
void quit(const char *arg);
void reapply(const char *arg);
void resize(Client *c, int x, int y, int w, int h, Bool sizehints);
void resizemouse(Client *c);
void restack(void);
void run(void);
void scan(void);
void setclientstate(Client *c, long state);
void setlayout(const char *arg);
void setmfact(const char *arg);
void setup(void);
void spawn(const char *arg);
void tag(const char *arg);
unsigned int textnw(const char *text, unsigned int len);
unsigned int textw(const char *text);
void tileh(void);
void tilehstack(unsigned int n);
Client *tilemaster(unsigned int n);
void tileresize(Client *c, int x, int y, int w, int h);
void tilev(void);
void tilevstack(unsigned int n);
void togglefloating(const char *arg);
void toggletag(const char *arg);
void toggleview(const char *arg);
void unban(Client *c);
void unmanage(Client *c);
void unmapnotify(XEvent *e);
void updatebar(void);
void updategeom(void);
void updatesizehints(Client *c);
void updatetitle(Client *c);
void updatewmhints(Client *c);
void view(const char *arg);
void viewprevtag(const char *arg);	/* views previous selected tags */
int xerror(Display *dpy, XErrorEvent *ee);
int xerrordummy(Display *dpy, XErrorEvent *ee);
int xerrorstart(Display *dpy, XErrorEvent *ee);
void zoom(const char *arg);

/* variables */
char stext[256];
int screen, sx, sy, sw, sh;
int (*xerrorxlib)(Display *, XErrorEvent *);
int bx, by, bw, bh, blw, mx, my, mw, mh, tx, ty, tw, th, wx, wy, ww, wh;
int seltags = 0;
double mfact;
unsigned int numlockmask = 0;
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
Bool otherwm, readin;
Bool running = True;
Bool *tagset[2];
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
#define TAGSZ (LENGTH(tags) * sizeof(Bool))

/* function implementations */

void
applyrules(Client *c) {
	unsigned int i;
	Bool matched = False;
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
			if(r->tag) {
				c->tags[idxoftag(r->tag)] = True;
				matched = True;
			}
		}
	}
	if(ch.res_class)
		XFree(ch.res_class);
	if(ch.res_name)
		XFree(ch.res_name);
	if(!matched)
		memcpy(c->tags, tagset[seltags], TAGSZ);
}

void
arrange(void) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(isvisible(c, NULL)) {
			unban(c);
			if(lt->isfloating || c->isfloating)
				resize(c, c->fx, c->fy, c->fw, c->fh, True);
		}
		else
			ban(c);

	focus(NULL);
	if(lt->arrange)
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
ban(Client *c) {
	if(c->isbanned)
		return;
	XMoveWindow(dpy, c->win, c->x + 2 * sw, c->y);
	c->isbanned = True;
}

void
buttonpress(XEvent *e) {
	unsigned int i, x;
	Client *c;
	XButtonPressedEvent *ev = &e->xbutton;

	if(ev->window == barwin) {
		x = 0;
		for(i = 0; i < LENGTH(tags); i++) {
			x += textw(tags[i]);
			if(ev->x < x) {
				if(ev->button == Button1) {
					if(ev->state & MODKEY)
						tag(tags[i]);
					else
						view(tags[i]);
				}
				else if(ev->button == Button3) {
					if(ev->state & MODKEY)
						toggletag(tags[i]);
					else
						toggleview(tags[i]);
				}
				return;
			}
		}
		if((ev->x < x + blw) && ev->button == Button1) 
			setlayout(NULL);
	}
	else if((c = getclient(ev->window))) {
		focus(c);
		if(CLEANMASK(ev->state) != MODKEY)
			return;
		if(ev->button == Button1) {
			restack();
			movemouse(c);
		}
		else if(ev->button == Button2) {
			if(!lt->isfloating && c->isfloating)
				togglefloating(NULL);
			else
				zoom(NULL);
		}
		else if(ev->button == Button3 && !c->isfixed) {
			restack();
			resizemouse(c);
		}
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
	while(stack) {
		unban(stack);
		unmanage(stack);
	}
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
		if(c->isfixed || c->isfloating || lt->isfloating) {
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
			if(isvisible(c, NULL))
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

unsigned int
counttiled(void) {
	unsigned int n;
	Client *c;

	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next), n++);
	return n;
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

	for(tc=&stack; *tc && *tc != c; tc=&(*tc)->snext);
	*tc = c->snext;
}

void
drawbar(void) {
	int i, x;
	Client *c;

	dc.x = 0;
	for(c = stack; c && !isvisible(c, NULL); c = c->snext);
	for(i = 0; i < LENGTH(tags); i++) {
		dc.w = textw(tags[i]);
		if(tagset[seltags][i]) {
			drawtext(tags[i], dc.sel, isurgent(i));
			drawsquare(c && c->tags[i], isoccupied(i), isurgent(i), dc.sel);
		}
		else {
			drawtext(tags[i], dc.norm, isurgent(i));
			drawsquare(c && c->tags[i], isoccupied(i), isurgent(i), dc.norm);
		}
		dc.x += dc.w;
	}
	if(blw > 0) {
		dc.w = blw;
		drawtext(lt->symbol, dc.norm, False);
		x = dc.x + dc.w;
	}
	else
		x = dc.x;
	dc.w = textw(stext);
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
	int x, y, w, h;
	unsigned int len, olen;
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
	if (!len)
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

void *
emallocz(unsigned int size) {
	void *res = calloc(1, size);

	if(!res)
		eprint("fatal: could not malloc() %u bytes\n", size);
	return res;
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
	if(!c || (c && !isvisible(c, NULL)))
		for(c = stack; c && !isvisible(c, NULL); c = c->snext);
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
focusnext(const char *arg) {
	Client *c;

	if(!sel)
		return;
	for(c = sel->next; c && !isvisible(c, arg ? sel->tags : NULL); c = c->next);
	if(!c)
		for(c = clients; c && !isvisible(c, arg ? sel->tags : NULL); c = c->next);
	if(c) {
		focus(c);
		restack();
	}
}

void
focusprev(const char *arg) {
	Client *c;

	if(!sel)
		return;
	for(c = sel->prev; c && !isvisible(c, arg ? sel->tags : NULL); c = c->prev);
	if(!c) {
		for(c = clients; c && c->next; c = c->next);
		for(; c && !isvisible(c, arg ? sel->tags : NULL); c = c->prev);
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
		eprint("error, cannot allocate color '%s'\n", colstr);
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
	int i, j;
	unsigned int buttons[]   = { Button1, Button2, Button3 };
	unsigned int modifiers[] = { MODKEY, MODKEY|LockMask, MODKEY|numlockmask,
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
grabkeys(void)  {
	unsigned int i, j;
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

unsigned int
idxoftag(const char *t) {
	unsigned int i;

	for(i = 0; (i < LENGTH(tags)) && t && strcmp(tags[i], t); i++);
	return (i < LENGTH(tags)) ? i : 0;
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
isoccupied(unsigned int t) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(c->tags[t])
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
isurgent(unsigned int t) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(c->isurgent && c->tags[t])
			return True;
	return False;
}

Bool
isvisible(Client *c, Bool *cmp) {
	unsigned int i;

	if(!cmp)
		cmp = tagset[seltags];
	for(i = 0; i < LENGTH(tags); i++)
		if(c->tags[i] && cmp[i])
			return True;
	return False;
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
		&& CLEANMASK(keys[i].mod) == CLEANMASK(ev->state))
		{
			if(keys[i].func)
				keys[i].func(keys[i].arg);
		}
}

void
killclient(const char *arg) {
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

	c = emallocz(sizeof(Client));
	c->tags = emallocz(TAGSZ);
	c->win = w;

	/* geometry */
	c->x = wa->x;
	c->y = wa->y;
	c->w = c->fw = wa->width;
	c->h = c->fh = wa->height;
	c->oldbw = wa->border_width;
	if(c->w == sw && c->h == sh) {
		c->x = sx;
		c->y = sy;
		c->bw = wa->border_width;
	}
	else {
		if(c->x + c->w + 2 * c->bw > wx + ww)
			c->x = wx + ww - c->w - 2 * c->bw;
		if(c->y + c->h + 2 * c->bw > wy + wh)
			c->y = wy + wh - c->h - 2 * c->bw;
		c->x = MAX(c->x, wx);
		c->y = MAX(c->y, wy);
		c->bw = BORDERPX;
	}
	c->fx = c->x;
	c->fy = c->y;

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
		memcpy(c->tags, t->tags, TAGSZ);
	else
		applyrules(c);
	if(!c->isfloating)
		c->isfloating = (rettrans == Success) || c->isfixed;
	attach(c);
	attachstack(c);
	XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h); /* some windows require this */
	ban(c);
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

	for(c = clients; c; c = c->next)
		if((lt->isfloating || !c->isfloating) &&  isvisible(c, NULL))
			resize(c, wx, wy, ww - 2 * c->bw, wh - 2 * c->bw, RESIZEHINTS);
}

void
movemouse(Client *c) {
	int x1, y1, ocx, ocy, di, nx, ny;
	unsigned int dui;
	Window dummy;
	XEvent ev;

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
			if(abs(wx - nx) < SNAP)
				nx = wx;
			else if(abs((wx + ww) - (nx + c->w + 2 * c->bw)) < SNAP)
				nx = wx + ww - c->w - 2 * c->bw;
			if(abs(wy - ny) < SNAP)
				ny = wy;
			else if(abs((wy + wh) - (ny + c->h + 2 * c->bw)) < SNAP)
				ny = wy + wh - c->h - 2 * c->bw;
			if(!c->isfloating && !lt->isfloating && (abs(nx - c->x) > SNAP || abs(ny - c->y) > SNAP))
				togglefloating(NULL);
			if(lt->isfloating || c->isfloating) {
				c->fx = nx;
				c->fy = ny;
				resize(c, nx, ny, c->w, c->h, False);
			}
			break;
		}
	}
}

Client *
nexttiled(Client *c) {
	for(; c && (c->isfloating || !isvisible(c, NULL)); c = c->next);
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
quit(const char *arg) {
	readin = running = False;
}

void
reapply(const char *arg) {
	Client *c;

	for(c = clients; c; c = c->next) {
		memset(c->tags, 0, TAGSZ);
		applyrules(c);
	}
	arrange();
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
		&& c->minax > 0 && c->maxax > 0 && c->minay > 0 && c->maxay > 0)
		{
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
resizemouse(Client *c) {
	int ocx, ocy;
	int nw, nh;
	XEvent ev;

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
			if(!c->isfloating && !lt->isfloating && (abs(nw - c->w) > SNAP || abs(nh - c->h) > SNAP)) {
				c->fx = c->x;
				c->fy = c->y;
				togglefloating(NULL);
			}
			if((lt->isfloating) || c->isfloating) {
				resize(c, c->x, c->y, nw, nh, True);
				c->fw = nw;
				c->fh = nh;
			}
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
	if(sel->isfloating || lt->isfloating)
		XRaiseWindow(dpy, sel->win);
	if(!lt->isfloating) {
		wc.stack_mode = Below;
		wc.sibling = barwin;
		for(c = stack; c; c = c->snext)
			if(!c->isfloating && isvisible(c, NULL)) {
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
	unsigned int i, num;
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

void
setlayout(const char *arg) {
	unsigned int i;

	if(!arg) {
		if(++lt == &layouts[LENGTH(layouts)])
			lt = &layouts[0];
	}
	else {
		for(i = 0; i < LENGTH(layouts); i++)
			if(!strcmp(arg, layouts[i].symbol))
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
setmfact(const char *arg) {
	double d;

	if(lt->isfloating)
		return;
	if(!arg)
		mfact = MFACT;
	else {
		d = strtod(arg, NULL);
		if(arg[0] == '-' || arg[0] == '+')
			d += mfact;
		if(d < 0.1 || d > 0.9)
			return;
		mfact = d;
	}
	updategeom();
	arrange();
}

void
setup(void) {
	unsigned int i, w;
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

	/* update geometry */
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

	/* init tags */
	tagset[0] = emallocz(TAGSZ);
	tagset[1] = emallocz(TAGSZ);
	tagset[0][0] = tagset[1][0] = True;

	/* init bar */
	for(blw = i = 0; LENGTH(layouts) > 1 && i < LENGTH(layouts); i++) {
		w = textw(layouts[i].symbol);
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
spawn(const char *arg) {
	static char *shell = NULL;

	if(!shell && !(shell = getenv("SHELL")))
		shell = "/bin/sh";
	if(!arg)
		return;
	/* The double-fork construct avoids zombie processes and keeps the code
	 * clean from stupid signal handlers. */
	if(fork() == 0) {
		if(fork() == 0) {
			if(dpy)
				close(ConnectionNumber(dpy));
			setsid();
			execl(shell, shell, "-c", arg, (char *)NULL);
			fprintf(stderr, "dwm: execl '%s -c %s'", shell, arg);
			perror(" failed");
		}
		exit(0);
	}
	wait(0);
}

void
tag(const char *arg) {
	unsigned int i;

	if(!sel)
		return;
	for(i = 0; i < LENGTH(tags); i++)
		sel->tags[i] = (NULL == arg);
	sel->tags[idxoftag(arg)] = True;
	arrange();
}

unsigned int
textnw(const char *text, unsigned int len) {
	XRectangle r;

	if(dc.font.set) {
		XmbTextExtents(dc.font.set, text, len, NULL, &r);
		return r.width;
	}
	return XTextWidth(dc.font.xfont, text, len);
}

unsigned int
textw(const char *text) {
	return textnw(text, strlen(text)) + dc.font.height;
}

void
tileh(void) {
	int x, w;
	unsigned int i, n = counttiled();
	Client *c;

	if(n == 0)
		return;
	c = tilemaster(n);
	if(--n == 0)
		return;

	x = tx;
	w = tw / n;
	if(w < bh)
		w = tw;

	for(i = 0, c = nexttiled(c->next); c; c = nexttiled(c->next), i++) {
		if(i + 1 == n) /* remainder */
			tileresize(c, x, ty, (tx + tw) - x - 2 * c->bw, th - 2 * c->bw);
		else
			tileresize(c, x, ty, w - 2 * c->bw, th - 2 * c->bw);
		if(w != tw)
			x = c->x + c->w + 2 * c->bw;
	}
}

Client *
tilemaster(unsigned int n) {
	Client *c = nexttiled(clients);

	if(n == 1)
		tileresize(c, wx, wy, ww - 2 * c->bw, wh - 2 * c->bw);
	else
		tileresize(c, mx, my, mw - 2 * c->bw, mh - 2 * c->bw);
	return c;
}

void
tileresize(Client *c, int x, int y, int w, int h) {
	resize(c, x, y, w, h, RESIZEHINTS);
	if((RESIZEHINTS) && ((c->h < bh) || (c->h > h) || (c->w < bh) || (c->w > w)))
		/* client doesn't accept size constraints */
		resize(c, x, y, w, h, False);
}

void
tilev(void) {
	int y, h;
	unsigned int i, n = counttiled();
	Client *c;

	if(n == 0)
		return;
	c = tilemaster(n);
	if(--n == 0)
		return;

	y = ty;
	h = th / n;
	if(h < bh)
		h = th;

	for(i = 0, c = nexttiled(c->next); c; c = nexttiled(c->next), i++) {
		if(i + 1 == n) /* remainder */
			tileresize(c, tx, y, tw - 2 * c->bw, (ty + th) - y - 2 * c->bw);
		else
			tileresize(c, tx, y, tw - 2 * c->bw, h - 2 * c->bw);
		if(h != th)
			y = c->y + c->h + 2 * c->bw;
	}
}

void
togglefloating(const char *arg) {
	if(!sel)
		return;
	sel->isfloating = !sel->isfloating;
	if(sel->isfloating)
		resize(sel, sel->x, sel->y, sel->w, sel->h, True);
	arrange();
}

void
toggletag(const char *arg) {
	unsigned int i, j;

	if(!sel)
		return;
	i = idxoftag(arg);
	sel->tags[i] = !sel->tags[i];
	for(j = 0; j < LENGTH(tags) && !sel->tags[j]; j++);
	if(j == LENGTH(tags))
		sel->tags[i] = True; /* at least one tag must be enabled */
	arrange();
}

void
toggleview(const char *arg) {
	unsigned int i, j;

	i = idxoftag(arg);
	tagset[seltags][i] = !tagset[seltags][i];
	for(j = 0; j < LENGTH(tags) && !tagset[seltags][j]; j++);
	if(j == LENGTH(tags))
		tagset[seltags][i] = True; /* at least one tag must be viewed */
	arrange();
}

void
unban(Client *c) {
	if(!c->isbanned)
		return;
	XMoveWindow(dpy, c->win, c->x, c->y);
	c->isbanned = False;
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
	free(c->tags);
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

	/* bar geometry */
	bx = 0;
	by = 0;
	bw = sw;

	/* window area geometry */
	wx = sx;
	wy = sy;
	ww = sw;
	sh = sh - bh;

	/* master area geometry */
	mfact = MFACT;
	mx = wx;
	my = wy;
	mw = mfact * ww;
	mh = wh;

	/* tile area geometry */
	tx = mx + mw;
	ty = wy;
	tw = ww - mw;
	th = wh;
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
view(const char *arg) {
	seltags ^= 1;
	memset(tagset[seltags], (NULL == arg), TAGSZ);
	tagset[seltags][idxoftag(arg)] = True;
	arrange();
}

void
viewprevtag(const char *arg) {
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
zoom(const char *arg) {
	Client *c = sel;

	if(c == nexttiled(clients))
		if(!c || !(c = nexttiled(c->next)))
			return;
	if(!lt->isfloating && !sel->isfloating) {
		detach(c);
		attach(c);
		focus(c);
	}
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
