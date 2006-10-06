/* (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
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
 * global tags array to indicate the tags of a client.  For each client dwm
 * creates a small title window, which is resized whenever the (_NET_)WM_NAME
 * properties are updated or the client is moved/resized.
 *
 * Keys and tagging rules are organized as arrays and defined in the config.h
 * file. These arrays are kept static in event.o and tag.o respectively,
 * because no other part of dwm needs access to them.  The current mode is
 * represented by the arrange() function pointer, which wether points to
 * dofloat() or dotile(). 
 *
 * To understand everything else, start reading main.c:main().
 */

#include "config.h"
#include <X11/Xlib.h>

/* mask shorthands, used in event.c and client.c */
#define BUTTONMASK		(ButtonPressMask | ButtonReleaseMask)
#define MOUSEMASK		(BUTTONMASK | PointerMotionMask)
/* other stuff used in different places */
#define BORDERPX		1
#define PROTODELWIN		1

enum { NetSupported, NetWMName, NetLast };		/* EWMH atoms */
enum { WMProtocols, WMDelete, WMLast };			/* default atoms */
enum { CurNormal, CurResize, CurMove, CurLast };	/* cursor */
enum { ColFG, ColBG, ColLast };				/* color */

typedef enum {
	TopLeft, TopRight, BotLeft, BotRight
} Corner; /* window corners */

typedef union {
	const char *cmd;
	int i;
} Arg; /* argument type */

typedef struct {
	int ascent;
	int descent;
	int height;
	XFontSet set;
	XFontStruct *xfont;
} Fnt;

typedef struct {
	int x, y, w, h;
	unsigned long norm[ColLast];
	unsigned long sel[ColLast];
	unsigned long status[ColLast];
	Drawable drawable;
	Fnt font;
	GC gc;
} DC; /* draw context */

typedef struct Client Client;
struct Client {
	char name[256];
	int proto;
	int x, y, w, h;
	int rx, ry, rw, rh; /* revert geometry */
	int tx, ty, tw, th; /* title window geometry */
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int grav;
	long flags; 
	unsigned int border, weight;
	Bool isfloat, ismax;
	Bool *tags;
	Client *next;
	Client *prev;
	Client *snext;
	Window win;
	Window twin;
};

extern const char *tags[];			/* all tags */
extern char stext[1024];			/* status text */
extern int bx, by, bw, bh, bmw;			/* bar geometry, bar mode label width */
extern int screen, sx, sy, sw, sh;		/* screen geometry */
extern unsigned int master, ntags, numlockmask;	/* master percent, number of tags, dynamic lock mask */
extern void (*handler[LASTEvent])(XEvent *);	/* event handler */
extern void (*arrange)(void);			/* arrange function, indicates mode  */
extern Atom wmatom[WMLast], netatom[NetLast];
extern Bool running, issel, *seltag;		/* seltag is array of Bool */
extern Client *clients, *sel, *stack;		/* global client list and stack */
extern Cursor cursor[CurLast];
extern DC dc;					/* global draw context */
extern Display *dpy;
extern Window root, barwin;

/* client.c */
extern void ban(Client *c);			/* ban c from screen */
extern void configure(Client *c);		/* send synthetic configure event */
extern void focus(Client *c);			/* focus c, c may be NULL */
extern Client *getclient(Window w);		/* return client of w */
extern Client *getctitle(Window w);		/* return client of title window */
extern void gravitate(Client *c, Bool invert);	/* gravitate c */
extern void killclient(Arg *arg);		/* kill c nicely */
extern void manage(Window w, XWindowAttributes *wa);	/* manage new client */
extern void resize(Client *c, Bool sizehints, Corner sticky); /* resize c*/
extern void resizetitle(Client *c);		/* resizes c->twin correctly */
extern void updatesize(Client *c);			/* update the size structs of c */
extern void updatetitle(Client *c);		/* update the name of c */
extern void unmanage(Client *c);		/* destroy c */

/* draw.c */
extern void drawall(void);			/* draw all visible client titles and the bar */
extern void drawstatus(void);			/* draw the bar */
extern void drawtitle(Client *c);		/* draw title of c */
extern unsigned long getcolor(const char *colstr);	/* return color of colstr */
extern void setfont(const char *fontstr);	/* set the font for DC */
extern unsigned int textw(const char *text);	/* return the width of text in px*/

/* event.c */
extern void grabkeys(void);			/* grab all keys defined in config.h */
extern void procevent(void);			/* process pending X events */

/* main.c */
extern int getproto(Window w);			/* return protocol mask of WMProtocols property of w */
extern void quit(Arg *arg);			/* quit dwm nicely */
extern void sendevent(Window w, Atom a, long value);	/* send synthetic event to w */
extern int xerror(Display *dsply, XErrorEvent *ee);	/* dwm's X error handler */

/* tag.c */
extern void initrregs(void);			/* initialize regexps of rules defined in config.h */
extern Client *getnext(Client *c);		/* returns next visible client */
extern Client *getprev(Client *c);		/* returns previous visible client */
extern void settags(Client *c, Client *trans);	/* sets tags of c */
extern void tag(Arg *arg);			/* tags c with arg's index */
extern void toggletag(Arg *arg);		/* toggles c tags with arg's index */

/* util.c */
extern void *emallocz(unsigned int size);	/* allocates zero-initialized memory, exits on error */
extern void eprint(const char *errstr, ...);	/* prints errstr and exits with 1 */
extern void *erealloc(void *ptr, unsigned int size);	/* reallocates memory, exits on error */
extern void spawn(Arg *arg);			/* forks a new subprocess with to arg's cmd */

/* view.c */
extern void detach(Client *c);			/* detaches c from global client list */
extern void dofloat(void);			/* arranges all windows floating */
extern void dotile(void);			/* arranges all windows tiled */
extern void focusnext(Arg *arg);		/* focuses next visible client, arg is ignored  */
extern void focusprev(Arg *arg);		/* focuses previous visible client, arg is ignored */
extern Bool isvisible(Client *c);		/* returns True if client is visible */
extern void resizecol(Arg *arg);		/* resizes the master percent with arg's index value */
extern void restack(void);			/* restores z layers of all clients */
extern void togglemode(Arg *arg);		/* toggles global arrange function (dotile/dofloat) */
extern void toggleview(Arg *arg);		/* toggles the tag with arg's index (in)visible */
extern void view(Arg *arg);			/* views the tag with arg's index */
extern void viewall(Arg *arg);			/* views all tags, arg is ignored */
extern void zoom(Arg *arg);			/* zooms the focused client to master area, arg is ignored */
