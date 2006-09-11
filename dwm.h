/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "config.h"
#include <X11/Xlib.h>

/* mask shorthands, used in event.c and client.c */
#define BUTTONMASK		(ButtonPressMask | ButtonReleaseMask)
#define MOUSEMASK		(BUTTONMASK | PointerMotionMask)
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
	int tx, ty, tw, th; /* title window geometry */
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int grav;
	long flags; 
	unsigned int border, weight;
	Bool isfloat;
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
extern int mw, screen, sx, sy, sw, sh;		/* screen geometry, master width */
extern unsigned int ntags, numlockmask;		/* number of tags, and dynamic lock mask */
extern void (*handler[LASTEvent])(XEvent *);	/* event handler */
extern void (*arrange)(Arg *);			/* arrange function, indicates mode  */
extern Atom wmatom[WMLast], netatom[NetLast];
extern Bool running, issel, maximized, *seltag;	/* seltag is array of Bool */
extern Client *clients, *sel, *stack;		/* Client containers */
extern Cursor cursor[CurLast];
extern DC dc;					/* draw context for everything */
extern Display *dpy;
extern Window root, barwin;

/* client.c */
extern void ban(Client *c);			/* ban client from screen */
extern void focus(Client *c);			/* focus c, c may be NULL */
extern Client *getclient(Window w);		/* return client of w */
extern Client *getctitle(Window w);		/* return client of title window */
extern void gravitate(Client *c, Bool invert);	/* gravitate c */
extern void killclient(Arg *arg);		/* kill c nicely */
extern void manage(Window w, XWindowAttributes *wa);	/* manage new client */
extern void resize(Client *c, Bool sizehints, Corner sticky); /* resize c*/
extern void setsize(Client *c);			/* set the size structs of c */
extern void settitle(Client *c);		/* set the name of c */
extern void togglemax(Arg *arg);		/* (un)maximize c */
extern void unmanage(Client *c);		/* destroy c */

/* draw.c */
extern void drawall();				/* draw all visible client titles and the bar */
extern void drawstatus();			/* draw the bar */
extern void drawtitle(Client *c);		/* draw title of c */
extern unsigned long getcolor(const char *colstr);	/* return color of colstr */
extern void setfont(const char *fontstr);	/* set the font for DC */
extern unsigned int textw(const char *text);	/* return the text width of text */

/* event.c */
extern void grabkeys();				/* grab all keys defined in config.h */
extern void procevent();			/* process pending X events */

/* main.c */
extern int getproto(Window w);			/* return protocol mask of WMProtocols property of w */
extern void quit(Arg *arg);			/* quit dwm nicely */
extern void sendevent(Window w, Atom a, long value);	/* send synthetic event to w */
extern int xerror(Display *dsply, XErrorEvent *ee);	/* dwm's X error handler */

/* tag.c */
extern void initrregs();			/* initialize regexps of rules defined in config.h */
extern Client *getnext(Client *c);		/* returns next visible client */
extern Client *getprev(Client *c);		/* returns previous visible client */
extern void settags(Client *c, Client *trans);	/* updates tags of c */
extern void tag(Arg *arg);			/* tags c accordingly to arg's index */
extern void toggletag(Arg *arg);		/* toggles c tags accordingly to arg's index */

/* util.c */
extern void *emallocz(unsigned int size);	/* allocates zero-initialized memory, exits on error */
extern void eprint(const char *errstr, ...);	/* prints error string and exits with return code 1 */
extern void *erealloc(void *ptr, unsigned int size);	/* reallocates memory, exits on error */
extern void spawn(Arg *arg);			/* forks a new subprocess accordingly to arg's cmd */

/* view.c */
extern void detach(Client *c);			/* detaches c from global client list */
extern void dofloat(Arg *arg);			/* arranges all windows in a floating way, arg is ignored */
extern void dotile(Arg *arg);			/* arranges all windows in a tiled way, arg is ignored */
extern void focusnext(Arg *arg);		/* focuses next visible client, arg is ignored  */
extern void focusprev(Arg *arg);		/* focuses previous visible client, arg is ignored */
extern Bool isvisible(Client *c);		/* returns True if client is visible */
extern void resizecol(Arg *arg);		/* resizes the master width accordingly to arg's index value */
extern void restack();				/* restores z layers of all clients */
extern void togglemode(Arg *arg);		/* toggles global arrange mode (between dotile and dofloat) */
extern void toggleview(Arg *arg);		/* makes the tag accordingly to arg's index (in)visible */
extern void view(Arg *arg);			/* makes the tag accordingly to arg's index visible */
extern void viewall(Arg *arg);			/* makes all tags visible, arg is ignored */
extern void zoom(Arg *arg);			/* zooms the focused client to master column, arg is ignored */
