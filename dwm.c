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
#include <regex.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
//#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
//#endif

/* macros */
#define BUTTONMASK		(ButtonPressMask | ButtonReleaseMask)
#define CLEANMASK(mask)		(mask & ~(numlockmask | LockMask))
#define LENGTH(x)		(sizeof x / sizeof x[0])
#define MAXTAGLEN		16
#define MOUSEMASK		(BUTTONMASK | PointerMotionMask)


/* enums */
enum { BarTop, BarBot, BarOff };			/* bar position */
enum { CurNormal, CurResize, CurMove, CurLast };	/* cursor */
enum { ColBorder, ColFG, ColBG, ColLast };		/* color */
enum { NetSupported, NetWMName, NetLast };		/* EWMH atoms */
enum { WMProtocols, WMDelete, WMName, WMState, WMLast };/* default atoms */

/* typedefs */
typedef struct Client Client;
struct Client {
	char name[256];
	int x, y, w, h;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int minax, maxax, minay, maxay;
	long flags;
	unsigned int border, oldborder;
	Bool isbanned, isfixed, isfloating, isurgent;
	Bool *tags;
	Client *next;
	Client *prev;
	Client *snext;
	Window win;
	int monitor;
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
} Layout;

typedef struct {
	const char *prop;
	const char *tags;
	Bool isfloating;
	int monitor;
} Rule;

typedef struct {
	regex_t *propregex;
	regex_t *tagregex;
} Regs;

typedef struct {
	int screen;
	Window root;
	Window barwin;
	int sx, sy, sw, sh, wax, way, wah, waw;
	DC dc;
	Bool *seltags;
	Bool *prevtags;
	Layout *layout;
	double mwfact;
} Monitor;

/* function declarations */
void applyrules(Client *c);
void arrange(void);
void attach(Client *c);
void attachstack(Client *c);
void ban(Client *c);
void buttonpress(XEvent *e);
void checkotherwm(void);
void cleanup(void);
void compileregs(void);
void configure(Client *c);
void configurenotify(XEvent *e);
void configurerequest(XEvent *e);
void destroynotify(XEvent *e);
void detach(Client *c);
void detachstack(Client *c);
void drawbar(void);
void drawsquare(Monitor *, Bool filled, Bool empty, unsigned long col[ColLast]);
void drawtext(Monitor *, const char *text, unsigned long col[ColLast], Bool invert);
void *emallocz(unsigned int size);
void enternotify(XEvent *e);
void eprint(const char *errstr, ...);
void expose(XEvent *e);
void floating(void); /* default floating layout */
void focus(Client *c);
void focusin(XEvent *e);
void focusnext(const char *arg);
void focusprev(const char *arg);
Client *getclient(Window w);
unsigned long getcolor(const char *colstr, int screen);
long getstate(Window w);
Bool gettextprop(Window w, Atom atom, char *text, unsigned int size);
void grabbuttons(Client *c, Bool focused);
void grabkeys(void);
unsigned int idxoftag(const char *tag);
void initfont(Monitor*, const char *fontstr);
Bool isoccupied(Monitor *m, unsigned int t);
Bool isprotodel(Client *c);
Bool isurgent(int monitor, unsigned int t);
Bool isvisible(Client *c, int monitor);
void keypress(XEvent *e);
void killclient(const char *arg);
void manage(Window w, XWindowAttributes *wa);
void mappingnotify(XEvent *e);
void maprequest(XEvent *e);
void movemouse(Client *c);
Client *nexttiled(Client *c, int monitor);
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
void setmwfact(const char *arg);
void setup(void);
void spawn(const char *arg);
void tag(const char *arg);
unsigned int textnw(Monitor*, const char *text, unsigned int len);
unsigned int textw(Monitor*, const char *text);
void tile(void);
void togglebar(const char *arg);
void togglefloating(const char *arg);
void toggletag(const char *arg);
void toggleview(const char *arg);
void unban(Client *c);
void unmanage(Client *c);
void unmapnotify(XEvent *e);
void updatebarpos(Monitor *m);
void updatesizehints(Client *c);
void updatetitle(Client *c);
void updatewmhints(Client *c);
void view(const char *arg);
void viewprevtag(const char *arg);	/* views previous selected tags */
int xerror(Display *dpy, XErrorEvent *ee);
int xerrordummy(Display *dsply, XErrorEvent *ee);
int xerrorstart(Display *dsply, XErrorEvent *ee);
void zoom(const char *arg);
int monitorat(void);
void movetomonitor(const char *arg);
void selectmonitor(const char *arg);

/* variables */
char stext[256];
int mcount = 1;
int (*xerrorxlib)(Display *, XErrorEvent *);
unsigned int bh, bpos;
unsigned int blw = 0;
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
Bool isxinerama = False;
Bool domwfact = True;
Bool dozoom = True;
Bool otherwm, readin;
Bool running = True;
Client *clients = NULL;
Client *sel = NULL;
Client *stack = NULL;
Cursor cursor[CurLast];
Display *dpy;
DC dc = {0};
Regs *regs = NULL;
Monitor *monitors;
int selmonitor = 0;

/* configuration, allows nested code to access above variables */
#include "config.h"

//Bool prevtags[LENGTH(tags)];

/* function implementations */
void
applyrules(Client *c) {
	static char buf[512];
	unsigned int i, j;
	regmatch_t tmp;
	Bool matched_tag = False;
	Bool matched_monitor = False;
	XClassHint ch = { 0 };

	/* rule matching */
	XGetClassHint(dpy, c->win, &ch);
	snprintf(buf, sizeof buf, "%s:%s:%s",
			ch.res_class ? ch.res_class : "",
			ch.res_name ? ch.res_name : "", c->name);
	for(i = 0; i < LENGTH(rules); i++)
		if(regs[i].propregex && !regexec(regs[i].propregex, buf, 1, &tmp, 0)) {
			if (rules[i].monitor >= 0 && rules[i].monitor < mcount) {
				matched_monitor = True;
				c->monitor = rules[i].monitor;
			}

			c->isfloating = rules[i].isfloating;
			for(j = 0; regs[i].tagregex && j < LENGTH(tags); j++) {
				if(!regexec(regs[i].tagregex, tags[j], 1, &tmp, 0)) {
					matched_tag = True;
					c->tags[j] = True;
				}
			}
		}
	if(ch.res_class)
		XFree(ch.res_class);
	if(ch.res_name)
		XFree(ch.res_name);
	if(!matched_tag)
		memcpy(c->tags, monitors[monitorat()].seltags, sizeof initags);
	if (!matched_monitor)
		c->monitor = monitorat();
}

void
arrange(void) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(isvisible(c, c->monitor))
			unban(c);
		else
			ban(c);

	monitors[selmonitor].layout->arrange();
	focus(NULL);
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
	XMoveWindow(dpy, c->win, c->x + 3 * monitors[c->monitor].sw, c->y);
	c->isbanned = True;
}

void
buttonpress(XEvent *e) {
	unsigned int i, x;
	Client *c;
	XButtonPressedEvent *ev = &e->xbutton;

	Monitor *m = &monitors[monitorat()];

	if(ev->window == m->barwin) {
		x = 0;
		for(i = 0; i < LENGTH(tags); i++) {
			x += textw(m, tags[i]);
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
			if((floating != m->layout->arrange) && c->isfloating)
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
	unsigned int i;
	close(STDIN_FILENO);
	while(stack) {
		unban(stack);
		unmanage(stack);
	}
	for(i = 0; i < mcount; i++) {
		Monitor *m = &monitors[i];
		if(m->dc.font.set)
			XFreeFontSet(dpy, m->dc.font.set);
		else
			XFreeFont(dpy, m->dc.font.xfont);
		XUngrabKey(dpy, AnyKey, AnyModifier, m->root);
		XFreePixmap(dpy, m->dc.drawable);
		XFreeGC(dpy, m->dc.gc);
		XDestroyWindow(dpy, m->barwin);
		XFreeCursor(dpy, cursor[CurNormal]);
		XFreeCursor(dpy, cursor[CurResize]);
		XFreeCursor(dpy, cursor[CurMove]);
		XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
		XSync(dpy, False);
	}
}

void
compileregs(void) {
	unsigned int i;
	regex_t *reg;

	if(regs)
		return;
	regs = emallocz(LENGTH(rules) * sizeof(Regs));
	for(i = 0; i < LENGTH(rules); i++) {
		if(rules[i].prop) {
			reg = emallocz(sizeof(regex_t));
			if(regcomp(reg, rules[i].prop, REG_EXTENDED))
				free(reg);
			else
				regs[i].propregex = reg;
		}
		if(rules[i].tags) {
			reg = emallocz(sizeof(regex_t));
			if(regcomp(reg, rules[i].tags, REG_EXTENDED))
				free(reg);
			else
				regs[i].tagregex = reg;
		}
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
	ce.border_width = c->border;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void
configurenotify(XEvent *e) {
	XConfigureEvent *ev = &e->xconfigure;
	Monitor *m = &monitors[selmonitor];

	if(ev->window == m->root && (ev->width != m->sw || ev->height != m->sh)) {
		m->sw = ev->width;
		m->sh = ev->height;
		XFreePixmap(dpy, dc.drawable);
		dc.drawable = XCreatePixmap(dpy, m->root, m->sw, bh, DefaultDepth(dpy, m->screen));
		XResizeWindow(dpy, m->barwin, m->sw, bh);
		updatebarpos(m);
		arrange();
	}
}

void
configurerequest(XEvent *e) {
	Client *c;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;

	if((c = getclient(ev->window))) {
		Monitor *m = &monitors[c->monitor];
		if(ev->value_mask & CWBorderWidth)
			c->border = ev->border_width;
		if(c->isfixed || c->isfloating || (floating == m->layout->arrange)) {
			if(ev->value_mask & CWX)
				c->x = m->sx+ev->x;
			if(ev->value_mask & CWY)
				c->y = m->sy+ev->y;
			if(ev->value_mask & CWWidth)
				c->w = ev->width;
			if(ev->value_mask & CWHeight)
				c->h = ev->height;
			if((c->x - m->sx + c->w) > m->sw && c->isfloating)
				c->x = m->sx + (m->sw / 2 - c->w / 2); /* center in x direction */
			if((c->y - m->sy + c->h) > m->sh && c->isfloating)
				c->y = m->sy + (m->sh / 2 - c->h / 2); /* center in y direction */
			if((ev->value_mask & (CWX | CWY))
			&& !(ev->value_mask & (CWWidth | CWHeight)))
				configure(c);
			if(isvisible(c, monitorat()))
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

	for(tc=&stack; *tc && *tc != c; tc=&(*tc)->snext);
	*tc = c->snext;
}

void
drawbar(void) {
	int i, j, x;

	for(i = 0; i < mcount; i++) {
		Monitor *m = &monitors[i];
		m->dc.x = 0;
		for(j = 0; j < LENGTH(tags); j++) {
			m->dc.w = textw(m, tags[j]);
			if(m->seltags[j]) {
				drawtext(m, tags[j], m->dc.sel, isurgent(i, j));
				drawsquare(m, sel && sel->tags[j] && sel->monitor == selmonitor, isoccupied(m, j), m->dc.sel);
			}
			else {
				drawtext(m, tags[j], m->dc.norm, isurgent(i, j));
				drawsquare(m, sel && sel->tags[j] && sel->monitor == selmonitor, isoccupied(m, j), m->dc.norm);
			}
			m->dc.x += m->dc.w;
		}
		m->dc.w = blw;
		drawtext(m, m->layout->symbol, m->dc.norm, False);
		x = m->dc.x + m->dc.w;
		m->dc.w = textw(m, stext);
		m->dc.x = m->sw - m->dc.w;
		if(m->dc.x < x) {
			m->dc.x = x;
			m->dc.w = m->sw - x;
		}
		drawtext(m, stext, m->dc.norm, False);
		if((m->dc.w = m->dc.x - x) > bh) {
			m->dc.x = x;
			if(sel && sel->monitor == selmonitor) {
				drawtext(m, sel->name, m->dc.sel, False);
				drawsquare(m, False, sel->isfloating, m->dc.sel);
			}
			else
				drawtext(m, NULL, m->dc.norm, False);
		}
		XCopyArea(dpy, m->dc.drawable, m->barwin, m->dc.gc, 0, 0, m->sw, bh, 0, 0);
		XSync(dpy, False);
	}
}

void
drawsquare(Monitor *m, Bool filled, Bool empty, unsigned long col[ColLast]) {
	int x;
	XGCValues gcv;
	XRectangle r = { m->dc.x, m->dc.y, m->dc.w, m->dc.h };

	gcv.foreground = col[ColFG];
	XChangeGC(dpy, m->dc.gc, GCForeground, &gcv);
	x = (m->dc.font.ascent + m->dc.font.descent + 2) / 4;
	r.x = m->dc.x + 1;
	r.y = m->dc.y + 1;
	if(filled) {
		r.width = r.height = x + 1;
		XFillRectangles(dpy, m->dc.drawable, m->dc.gc, &r, 1);
	}
	else if(empty) {
		r.width = r.height = x;
		XDrawRectangles(dpy, m->dc.drawable, m->dc.gc, &r, 1);
	}
}

void
drawtext(Monitor *m, const char *text, unsigned long col[ColLast], Bool invert) {
	int x, y, w, h;
	static char buf[256];
	unsigned int len, olen;
	XRectangle r = { m->dc.x, m->dc.y, m->dc.w, m->dc.h };

	XSetForeground(dpy, m->dc.gc, col[invert ? ColFG : ColBG]);
	XFillRectangles(dpy, m->dc.drawable, m->dc.gc, &r, 1);
	if(!text)
		return;
	w = 0;
	olen = len = strlen(text);
	if(len >= sizeof buf)
		len = sizeof buf - 1;
	memcpy(buf, text, len);
	buf[len] = 0;
	h = m->dc.font.ascent + m->dc.font.descent;
	y = m->dc.y + (m->dc.h / 2) - (h / 2) + m->dc.font.ascent;
	x = m->dc.x + (h / 2);
	/* shorten text if necessary */
	while(len && (w = textnw(m, buf, len)) > m->dc.w - h)
		buf[--len] = 0;
	if(len < olen) {
		if(len > 1)
			buf[len - 1] = '.';
		if(len > 2)
			buf[len - 2] = '.';
		if(len > 3)
			buf[len - 3] = '.';
	}
	if(w > m->dc.w)
		return; /* too long */
	XSetForeground(dpy, m->dc.gc, col[invert ? ColBG : ColFG]);
	if(m->dc.font.set)
		XmbDrawString(dpy, m->dc.drawable, m->dc.font.set, m->dc.gc, x, y, buf, len);
	else
		XDrawString(dpy, m->dc.drawable, m->dc.gc, x, y, buf, len);
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

	if(ev->mode != NotifyNormal || ev->detail == NotifyInferior);
		//return;
	if((c = getclient(ev->window)))
		focus(c);
	else {
		selmonitor = monitorat();
		fprintf(stderr, "updating selmonitor %d\n", selmonitor);
		focus(NULL);
	}
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

	if(ev->count == 0) {
		if(ev->window == monitors[selmonitor].barwin)
			drawbar();
	}
}

void
floating(void) { /* default floating layout */
	Client *c;

	domwfact = dozoom = False;
	for(c = clients; c; c = c->next)
		if(isvisible(c, selmonitor))
			resize(c, c->x, c->y, c->w, c->h, True);
}

void
focus(Client *c) {
	Monitor *m;

	if(c)
		selmonitor = c->monitor;
	m = &monitors[selmonitor];
	if(!c || (c && !isvisible(c, selmonitor)))
		for(c = stack; c && !isvisible(c, c->monitor); c = c->snext);
	if(sel && sel != c) {
		grabbuttons(sel, False);
		XSetWindowBorder(dpy, sel->win, monitors[sel->monitor].dc.norm[ColBorder]);
	}
	if(c) {
		detachstack(c);
		attachstack(c);
		grabbuttons(c, True);
	}
	sel = c;
	drawbar();
	if(c) {
		XSetWindowBorder(dpy, c->win, m->dc.sel[ColBorder]);
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		selmonitor = c->monitor;
	}
	else {
		XSetInputFocus(dpy, m->root, RevertToPointerRoot, CurrentTime);
	}
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
	for(c = sel->next; c && !isvisible(c, selmonitor); c = c->next);
	if(!c)
		for(c = clients; c && !isvisible(c, selmonitor); c = c->next);
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
	for(c = sel->prev; c && !isvisible(c, selmonitor); c = c->prev);
	if(!c) {
		for(c = clients; c && c->next; c = c->next);
		for(; c && !isvisible(c, selmonitor); c = c->prev);
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
getcolor(const char *colstr, int screen) {
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
	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);

	if(focused) {
		XGrabButton(dpy, Button1, MODKEY, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button1, MODKEY | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button1, MODKEY | numlockmask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button1, MODKEY | numlockmask | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);

		XGrabButton(dpy, Button2, MODKEY, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button2, MODKEY | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button2, MODKEY | numlockmask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button2, MODKEY | numlockmask | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);

		XGrabButton(dpy, Button3, MODKEY, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button3, MODKEY | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button3, MODKEY | numlockmask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button3, MODKEY | numlockmask | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
	}
	else
		XGrabButton(dpy, AnyButton, AnyModifier, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
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

	for(i = 0; i < mcount; i++) {
		Monitor *m = &monitors[i];
		XUngrabKey(dpy, AnyKey, AnyModifier, m->root);
		for(j = 0; j < LENGTH(keys); j++) {
			code = XKeysymToKeycode(dpy, keys[j].keysym);
			XGrabKey(dpy, code, keys[j].mod, m->root, True,
					GrabModeAsync, GrabModeAsync);
			XGrabKey(dpy, code, keys[j].mod | LockMask, m->root, True,
					GrabModeAsync, GrabModeAsync);
			XGrabKey(dpy, code, keys[j].mod | numlockmask, m->root, True,
					GrabModeAsync, GrabModeAsync);
			XGrabKey(dpy, code, keys[j].mod | numlockmask | LockMask, m->root, True,
					GrabModeAsync, GrabModeAsync);
		}
	}
}

unsigned int
idxoftag(const char *tag) {
	unsigned int i;

	for(i = 0; (i < LENGTH(tags)) && (tags[i] != tag); i++);
	return (i < LENGTH(tags)) ? i : 0;
}

void
initfont(Monitor *m, const char *fontstr) {
	char *def, **missing;
	int i, n;

	missing = NULL;
	if(m->dc.font.set)
		XFreeFontSet(dpy, m->dc.font.set);
	m->dc.font.set = XCreateFontSet(dpy, fontstr, &missing, &n, &def);
	if(missing) {
		while(n--)
			fprintf(stderr, "dwm: missing fontset: %s\n", missing[n]);
		XFreeStringList(missing);
	}
	if(m->dc.font.set) {
		XFontSetExtents *font_extents;
		XFontStruct **xfonts;
		char **font_names;
		m->dc.font.ascent = m->dc.font.descent = 0;
		font_extents = XExtentsOfFontSet(m->dc.font.set);
		n = XFontsOfFontSet(m->dc.font.set, &xfonts, &font_names);
		for(i = 0, m->dc.font.ascent = 0, m->dc.font.descent = 0; i < n; i++) {
			if(m->dc.font.ascent < (*xfonts)->ascent)
				m->dc.font.ascent = (*xfonts)->ascent;
			if(m->dc.font.descent < (*xfonts)->descent)
				m->dc.font.descent = (*xfonts)->descent;
			xfonts++;
		}
	}
	else {
		if(m->dc.font.xfont)
			XFreeFont(dpy, m->dc.font.xfont);
		m->dc.font.xfont = NULL;
		if(!(m->dc.font.xfont = XLoadQueryFont(dpy, fontstr))
		&& !(m->dc.font.xfont = XLoadQueryFont(dpy, "fixed")))
			eprint("error, cannot load font: '%s'\n", fontstr);
		m->dc.font.ascent = m->dc.font.xfont->ascent;
		m->dc.font.descent = m->dc.font.xfont->descent;
	}
	m->dc.font.height = m->dc.font.ascent + m->dc.font.descent;
}

Bool
isoccupied(Monitor *m, unsigned int t) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(c->tags[t] && c->monitor == selmonitor)
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
isurgent(int monitor, unsigned int t) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(c->monitor == monitor && c->isurgent && c->tags[t])
			return True;
	return False;
}

Bool
isvisible(Client *c, int monitor) {
	unsigned int i;

	for(i = 0; i < LENGTH(tags); i++)
		if(c->tags[i] && monitors[c->monitor].seltags[i] && c->monitor == monitor)
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
	Monitor *m;
	Status rettrans;
	Window trans;
	XWindowChanges wc;

	c = emallocz(sizeof(Client));
	c->tags = emallocz(sizeof initags);
	c->win = w;

	applyrules(c);

	m = &monitors[c->monitor];

	c->x = wa->x + m->sx;
	c->y = wa->y + m->sy;
	c->w = wa->width;
	c->h = wa->height;
	c->oldborder = wa->border_width;

	if(c->w == m->sw && c->h == m->sh) {
		c->x = m->sx;
		c->y = m->sy;
		c->border = wa->border_width;
	}
	else {
		if(c->x + c->w + 2 * c->border > m->wax + m->waw)
			c->x = m->wax + m->waw - c->w - 2 * c->border;
		if(c->y + c->h + 2 * c->border > m->way + m->wah)
			c->y = m->way + m->wah - c->h - 2 * c->border;
		if(c->x < m->wax)
			c->x = m->wax;
		if(c->y < m->way)
			c->y = m->way;
		c->border = BORDERPX;
	}
	wc.border_width = c->border;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, w, m->dc.norm[ColBorder]);
	configure(c); /* propagates border_width, if size doesn't change */
	updatesizehints(c);
	XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
	grabbuttons(c, False);
	updatetitle(c);
	if((rettrans = XGetTransientForHint(dpy, w, &trans) == Success))
		for(t = clients; t && t->win != trans; t = t->next);
	if(t)
		memcpy(c->tags, t->tags, sizeof initags);
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

int
monitorat() {
	int i, x, y;
	Window win;
	unsigned int mask;

	XQueryPointer(dpy, monitors[selmonitor].root, &win, &win, &x, &y, &i, &i, &mask);
	for(i = 0; i < mcount; i++) {
		fprintf(stderr, "checking monitor[%d]: %d %d %d %d\n", i, monitors[i].sx, monitors[i].sy, monitors[i].sw, monitors[i].sh);
		if((x >= monitors[i].sx && x < monitors[i].sx + monitors[i].sw)
		&& (y >= monitors[i].sy && y < monitors[i].sy + monitors[i].sh)) {
			fprintf(stderr, "%d,%d -> %d\n", x, y, i);
			return i;
		}
	}
	fprintf(stderr, "?,? -> 0\n");
	return 0;
}

void
movemouse(Client *c) {
	int x1, y1, ocx, ocy, di, nx, ny;
	unsigned int dui;
	Window dummy;
	XEvent ev;

	ocx = nx = c->x;
	ocy = ny = c->y;
	if(XGrabPointer(dpy, monitors[selmonitor].root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
			None, cursor[CurMove], CurrentTime) != GrabSuccess)
		return;
	XQueryPointer(dpy, monitors[selmonitor].root, &dummy, &dummy, &x1, &y1, &di, &di, &dui);
	for(;;) {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
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
			Monitor *m = &monitors[monitorat()];
			if(abs(m->wax - nx) < SNAP)
				nx = m->wax;
			else if(abs((m->wax + m->waw) - (nx + c->w + 2 * c->border)) < SNAP)
				nx = m->wax + m->waw - c->w - 2 * c->border;
			if(abs(m->way - ny) < SNAP)
				ny = m->way;
			else if(abs((m->way + m->wah) - (ny + c->h + 2 * c->border)) < SNAP)
				ny = m->way + m->wah - c->h - 2 * c->border;
			if((monitors[selmonitor].layout->arrange != floating) && (abs(nx - c->x) > SNAP || abs(ny - c->y) > SNAP))
				togglefloating(NULL);
			if((monitors[selmonitor].layout->arrange == floating) || c->isfloating)
				resize(c, nx, ny, c->w, c->h, False);
			memcpy(c->tags, monitors[monitorat()].seltags, sizeof initags);
			break;
		}
	}
}

Client *
nexttiled(Client *c, int monitor) {
	for(; c && (c->isfloating || !isvisible(c, monitor)); c = c->next);
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
	static Bool zerotags[LENGTH(tags)] = { 0 };
	Client *c;

	for(c = clients; c; c = c->next) {
		memcpy(c->tags, zerotags, sizeof zerotags);
		applyrules(c);
	}
	arrange();
}

void
resize(Client *c, int x, int y, int w, int h, Bool sizehints) {
	XWindowChanges wc;
	//Monitor scr = monitors[monitorat()];
//	c->monitor = monitorat();

	if(sizehints) {
		/* set minimum possible */
		if (w < 1)
			w = 1;
		if (h < 1)
			h = 1;

		/* temporarily remove base dimensions */
		w -= c->basew;
		h -= c->baseh;

		/* adjust for aspect limits */
		if (c->minay > 0 && c->maxay > 0 && c->minax > 0 && c->maxax > 0) {
			if (w * c->maxay > h * c->maxax)
				w = h * c->maxax / c->maxay;
			else if (w * c->minay < h * c->minax)
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

		if(c->minw > 0 && w < c->minw)
			w = c->minw;
		if(c->minh > 0 && h < c->minh)
			h = c->minh;
		if(c->maxw > 0 && w > c->maxw)
			w = c->maxw;
		if(c->maxh > 0 && h > c->maxh)
			h = c->maxh;
	}
	if(w <= 0 || h <= 0)
		return;
	/* TODO: offscreen appearance fixes */
	/*
	if(x > scr.sw)
		x = scr.sw - w - 2 * c->border;
	if(y > scr.sh)
		y = scr.sh - h - 2 * c->border;
	if(x + w + 2 * c->border < scr.sx)
		x = scr.sx;
	if(y + h + 2 * c->border < scr.sy)
		y = scr.sy;
	*/
	if(c->x != x || c->y != y || c->w != w || c->h != h) {
		c->x = wc.x = x;
		c->y = wc.y = y;
		c->w = wc.width = w;
		c->h = wc.height = h;
		wc.border_width = c->border;
		XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
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
	if(XGrabPointer(dpy, monitors[selmonitor].root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
			None, cursor[CurResize], CurrentTime) != GrabSuccess)
		return;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->border - 1, c->h + c->border - 1);
	for(;;) {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask , &ev);
		switch(ev.type) {
		case ButtonRelease:
			XWarpPointer(dpy, None, c->win, 0, 0, 0, 0,
					c->w + c->border - 1, c->h + c->border - 1);
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
			if((nw = ev.xmotion.x - ocx - 2 * c->border + 1) <= 0)
				nw = 1;
			if((nh = ev.xmotion.y - ocy - 2 * c->border + 1) <= 0)
				nh = 1;
			if((monitors[selmonitor].layout->arrange != floating) && (abs(nw - c->w) > SNAP || abs(nh - c->h) > SNAP))
				togglefloating(NULL);
			if((monitors[selmonitor].layout->arrange == floating) || c->isfloating)
				resize(c, c->x, c->y, nw, nh, True);
			break;
		}
	}
}

void
restack(void) {
	unsigned int i;
	Client *c;
	XEvent ev;
	XWindowChanges wc;

	drawbar();
	if(!sel)
		return;
	if(sel->isfloating || (monitors[selmonitor].layout->arrange == floating))
		XRaiseWindow(dpy, sel->win);
	if(monitors[selmonitor].layout->arrange != floating) {
		wc.stack_mode = Below;
		wc.sibling = monitors[selmonitor].barwin;
		if(!sel->isfloating) {
			XConfigureWindow(dpy, sel->win, CWSibling | CWStackMode, &wc);
			wc.sibling = sel->win;
		}
		for(i = 0; i < mcount; i++) {
			for(c = nexttiled(clients, i); c; c = nexttiled(c->next, i)) {
				if(c == sel)
					continue;
				XConfigureWindow(dpy, c->win, CWSibling | CWStackMode, &wc);
				wc.sibling = c->win;
			}
		}
	}
	XSync(dpy, False);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
run(void) {
	char *p;
	char buf[sizeof stext];
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
	buf[len] = stext[len] = '\0'; /* 0-terminator is never touched */
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
			switch((r = read(STDIN_FILENO, buf + offset, len - offset))) {
			case -1:
				strncpy(stext, strerror(errno), len);
				readin = False;
				break;
			case 0:
				strncpy(stext, "EOF", 4);
				readin = False;
				break;
			default:
				for(p = buf + offset; r > 0; p++, r--, offset++)
					if(*p == '\n' || *p == '\0') {
						*p = '\0';
						strncpy(stext, buf, len);
						p += r - 1; /* p is buf + offset + r - 1 */
						for(r = 0; *(p - r) && *(p - r) != '\n'; r++);
						offset = r;
						if(r)
							memmove(buf, p - r + 1, r);
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
	unsigned int i, j, num;
	Window *wins, d1, d2;
	XWindowAttributes wa;

	for(i = 0; i < mcount; i++) {
		Monitor *m = &monitors[i];
		wins = NULL;
		if(XQueryTree(dpy, m->root, &d1, &d2, &wins, &num)) {
			for(j = 0; j < num; j++) {
				if(!XGetWindowAttributes(dpy, wins[j], &wa)
				|| wa.override_redirect || XGetTransientForHint(dpy, wins[j], &d1))
					continue;
				if(wa.map_state == IsViewable || getstate(wins[j]) == IconicState)
					manage(wins[j], &wa);
			}
			for(j = 0; j < num; j++) { /* now the transients */
				if(!XGetWindowAttributes(dpy, wins[j], &wa))
					continue;
				if(XGetTransientForHint(dpy, wins[j], &d1)
				&& (wa.map_state == IsViewable || getstate(wins[j]) == IconicState))
					manage(wins[j], &wa);
			}
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
setlayout(const char *arg) {
	unsigned int i;
	Monitor *m = &monitors[monitorat()];

	if(!arg) {
		m->layout++;
		if(m->layout == &layouts[LENGTH(layouts)])
			m->layout = &layouts[0];
	}
	else {
		for(i = 0; i < LENGTH(layouts); i++)
			if(!strcmp(arg, layouts[i].symbol))
				break;
		if(i == LENGTH(layouts))
			return;
		m->layout = &layouts[i];
	}
	if(sel)
		arrange();
	else
		drawbar();
}

void
setmwfact(const char *arg) {
	double delta;

	Monitor *m = &monitors[monitorat()];

	if(!domwfact)
		return;
	/* arg handling, manipulate mwfact */
	if(arg == NULL)
		m->mwfact = MWFACT;
	else if(sscanf(arg, "%lf", &delta) == 1) {
		if(arg[0] == '+' || arg[0] == '-')
			m->mwfact += delta;
		else
			m->mwfact = delta;
		if(m->mwfact < 0.1)
			m->mwfact = 0.1;
		else if(m->mwfact > 0.9)
			m->mwfact = 0.9;
	}
	arrange();
}

void
setup(void) {
	unsigned int i, j, k;
	Monitor *m;
	XSetWindowAttributes wa;
	XineramaScreenInfo *info = NULL;

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

	// init screens/monitors first
	mcount = 1;
	if((isxinerama = XineramaIsActive(dpy)))
		info = XineramaQueryScreens(dpy, &mcount);
	monitors = emallocz(mcount * sizeof(Monitor));

	for(i = 0; i < mcount; i++) {
		/* init geometry */
		m = &monitors[i];

		m->screen = isxinerama ? 0 : i;
		m->root = RootWindow(dpy, m->screen);

		if (mcount != 1 && isxinerama) {
			m->sx = info[i].x_org;
			m->sy = info[i].y_org;
			m->sw = info[i].width;
			m->sh = info[i].height;
			fprintf(stderr, "monitor[%d]: %d,%d,%d,%d\n", i, m->sx, m->sy, m->sw, m->sh);
		}
		else {
			m->sx = 0;
			m->sy = 0;
			m->sw = DisplayWidth(dpy, m->screen);
			m->sh = DisplayHeight(dpy, m->screen);
		}

		m->seltags = emallocz(sizeof initags);
		m->prevtags = emallocz(sizeof initags);

		memcpy(m->seltags, initags, sizeof initags);
		memcpy(m->prevtags, initags, sizeof initags);

		/* init appearance */
		m->dc.norm[ColBorder] = getcolor(NORMBORDERCOLOR, m->screen);
		m->dc.norm[ColBG] = getcolor(NORMBGCOLOR, m->screen);
		m->dc.norm[ColFG] = getcolor(NORMFGCOLOR, m->screen);
		m->dc.sel[ColBorder] = getcolor(SELBORDERCOLOR, m->screen);
		m->dc.sel[ColBG] = getcolor(SELBGCOLOR, m->screen);
		m->dc.sel[ColFG] = getcolor(SELFGCOLOR, m->screen);
		initfont(m, FONT);
		m->dc.h = bh = m->dc.font.height + 2;

		/* init layouts */
		m->mwfact = MWFACT;
		m->layout = &layouts[0];
		for(blw = k = 0; k < LENGTH(layouts); k++) {
			j = textw(m, layouts[k].symbol);
			if(j > blw)
				blw = j;
		}

		// TODO: bpos per screen?
		bpos = BARPOS;
		wa.override_redirect = 1;
		wa.background_pixmap = ParentRelative;
		wa.event_mask = ButtonPressMask | ExposureMask;

		/* init bars */
		m->barwin = XCreateWindow(dpy, m->root, m->sx, m->sy, m->sw, bh, 0,
				DefaultDepth(dpy, m->screen), CopyFromParent, DefaultVisual(dpy, m->screen),
				CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
		XDefineCursor(dpy, m->barwin, cursor[CurNormal]);
		updatebarpos(m);
		XMapRaised(dpy, m->barwin);
		strcpy(stext, "dwm-"VERSION);
		m->dc.drawable = XCreatePixmap(dpy, m->root, m->sw, bh, DefaultDepth(dpy, m->screen));
		m->dc.gc = XCreateGC(dpy, m->root, 0, 0);
		XSetLineAttributes(dpy, m->dc.gc, 1, LineSolid, CapButt, JoinMiter);
		if(!m->dc.font.set)
			XSetFont(dpy, m->dc.gc, m->dc.font.xfont->fid);

		/* EWMH support per monitor */
		XChangeProperty(dpy, m->root, netatom[NetSupported], XA_ATOM, 32,
				PropModeReplace, (unsigned char *) netatom, NetLast);

		/* select for events */
		wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask
				| EnterWindowMask | LeaveWindowMask | StructureNotifyMask;
		XChangeWindowAttributes(dpy, m->root, CWEventMask | CWCursor, &wa);
		XSelectInput(dpy, m->root, wa.event_mask);
	}
	if(info)
		XFree(info);

	/* grab keys */
	grabkeys();

	/* init tags */
	compileregs();

	selmonitor = monitorat();
	fprintf(stderr, "selmonitor == %d\n", selmonitor);
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
textnw(Monitor *m, const char *text, unsigned int len) {
	XRectangle r;

	if(m->dc.font.set) {
		XmbTextExtents(m->dc.font.set, text, len, NULL, &r);
		return r.width;
	}
	return XTextWidth(m->dc.font.xfont, text, len);
}

unsigned int
textw(Monitor *m, const char *text) {
	return textnw(m, text, strlen(text)) + m->dc.font.height;
}

void
tile(void) {
	unsigned int i, j, n, nx, ny, nw, nh, mw, th;
	Client *c, *mc;

	domwfact = dozoom = True;

	nx = ny = nw = 0; /* gcc stupidity requires this */

	for (i = 0; i < mcount; i++) {
		Monitor *m = &monitors[i];

		for(n = 0, c = nexttiled(clients, i); c; c = nexttiled(c->next, i))
			n++;

		for(j = 0, c = mc = nexttiled(clients, i); c; c = nexttiled(c->next, i)) {
			/* window geoms */
			mw = (n == 1) ? m->waw : m->mwfact * m->waw;
			th = (n > 1) ? m->wah / (n - 1) : 0;
			if(n > 1 && th < bh)
				th = m->wah;
			if(j == 0) { /* master */
				nx = m->wax;
				ny = m->way;
				nw = mw - 2 * c->border;
				nh = m->wah - 2 * c->border;
			}
			else {  /* tile window */
				if(j == 1) {
					ny = m->way;
					nx += mc->w + 2 * mc->border;
					nw = m->waw - mw - 2 * c->border;
				}
				if(j + 1 == n) /* remainder */
					nh = (m->way + m->wah) - ny - 2 * c->border;
				else
					nh = th - 2 * c->border;
			}
			fprintf(stderr, "tile(%d, %d, %d, %d)\n", nx, ny, nw, nh);
			resize(c, nx, ny, nw, nh, RESIZEHINTS);
			if((RESIZEHINTS) && ((c->h < bh) || (c->h > nh) || (c->w < bh) || (c->w > nw)))
				/* client doesn't accept size constraints */
				resize(c, nx, ny, nw, nh, False);
			if(n > 1 && th != m->wah)
				ny = c->y + c->h + 2 * c->border;

			j++;
		}
	}
	fprintf(stderr, "done\n");
}
void
togglebar(const char *arg) {
	if(bpos == BarOff)
		bpos = (BARPOS == BarOff) ? BarTop : BARPOS;
	else
		bpos = BarOff;
	updatebarpos(&monitors[monitorat()]);
	arrange();
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

	Monitor *m = &monitors[monitorat()];

	i = idxoftag(arg);
	m->seltags[i] = !m->seltags[i];
	for(j = 0; j < LENGTH(tags) && !m->seltags[j]; j++);
	if(j == LENGTH(tags))
		m->seltags[i] = True; /* at least one tag must be viewed */
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

	wc.border_width = c->oldborder;
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
updatebarpos(Monitor *m) {
	XEvent ev;

	m->wax = m->sx;
	m->way = m->sy;
	m->wah = m->sh;
	m->waw = m->sw;
	switch(bpos) {
	default:
		m->wah -= bh;
		m->way += bh;
		XMoveWindow(dpy, m->barwin, m->sx, m->sy);
		break;
	case BarBot:
		m->wah -= bh;
		XMoveWindow(dpy, m->barwin, m->sx, m->sy + m->wah);
		break;
	case BarOff:
		XMoveWindow(dpy, m->barwin, m->sx, m->sy - bh);
		break;
	}
	XSync(dpy, False);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
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
		c->isurgent = (wmh->flags & XUrgencyHint) ? True : False;
		XFree(wmh);
	}
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
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
	fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
		ee->request_code, ee->error_code);
	return xerrorxlib(dpy, ee); /* may call exit */
}

int
xerrordummy(Display *dsply, XErrorEvent *ee) {
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int
xerrorstart(Display *dsply, XErrorEvent *ee) {
	otherwm = True;
	return -1;
}

void
view(const char *arg) {
	unsigned int i;
	Bool tmp[LENGTH(tags)];
	Monitor *m = &monitors[monitorat()];

	for(i = 0; i < LENGTH(tags); i++)
		tmp[i] = (NULL == arg);
	tmp[idxoftag(arg)] = True;
	if(memcmp(m->seltags, tmp, sizeof initags) != 0) {
		memcpy(m->prevtags, m->seltags, sizeof initags);
		memcpy(m->seltags, tmp, sizeof initags);
		arrange();
	}
}

void
viewprevtag(const char *arg) {
	static Bool tmp[LENGTH(tags)];

	Monitor *m = &monitors[monitorat()];

	memcpy(tmp, m->seltags, sizeof initags);
	memcpy(m->seltags, m->prevtags, sizeof initags);
	memcpy(m->prevtags, tmp, sizeof initags);
	arrange();
}

void
zoom(const char *arg) {
	Client *c;

	if(!sel || !dozoom || sel->isfloating)
		return;
	if((c = sel) == nexttiled(clients, c->monitor))
		if(!(c = nexttiled(c->next, c->monitor)))
			return;
	detach(c);
	attach(c);
	focus(c);
	arrange();
}

void
movetomonitor(const char *arg) {
	if (sel) {
		sel->monitor = arg ? atoi(arg) : (sel->monitor+1) % mcount;

		memcpy(sel->tags, monitors[sel->monitor].seltags, sizeof initags);
		resize(sel, monitors[sel->monitor].wax, monitors[sel->monitor].way, sel->w, sel->h, True);
		arrange();
	}
}

void
selectmonitor(const char *arg) {
	Monitor *m = &monitors[arg ? atoi(arg) : (monitorat()+1) % mcount];

	XWarpPointer(dpy, None, m->root, 0, 0, 0, 0, m->wax+m->waw/2, m->way+m->wah/2);
	focus(NULL);
}


int
main(int argc, char *argv[]) {
	if(argc == 2 && !strcmp("-v", argv[1]))
		eprint("dwm-"VERSION", © 2006-2007 Anselm R. Garbe, Sander van Dijk, "
		       "Jukka Salmi, Premysl Hruby, Szabolcs Nagy, Christof Musik\n");
	else if(argc != 1)
		eprint("usage: dwm [-v]\n");

	setlocale(LC_CTYPE, "");
	if(!(dpy = XOpenDisplay(0)))
		eprint("dwm: cannot open display\n");

	checkotherwm();
	setup();
	drawbar();
	scan();
	run();
	cleanup();

	XCloseDisplay(dpy);
	return 0;
}
