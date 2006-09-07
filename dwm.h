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

typedef union {
	const char *cmd;
	int i;
} Arg;

/* atoms */
enum { NetSupported, NetWMName, NetLast };
enum { WMProtocols, WMDelete, WMLast };

/* cursor */
enum { CurNormal, CurResize, CurMove, CurLast };

/* color */
enum { ColFG, ColBG, ColLast };

/* window corners */
typedef enum { TopLeft, TopRight, BotLeft, BotRight } Corner;

typedef struct {
	int ascent;
	int descent;
	int height;
	XFontSet set;
	XFontStruct *xfont;
} Fnt;

typedef struct { /* draw context */
	int x, y, w, h;
	unsigned long norm[ColLast];
	unsigned long sel[ColLast];
	unsigned long status[ColLast];
	Drawable drawable;
	Fnt font;
	GC gc;
} DC;

typedef struct Client Client;
struct Client {
	char name[256];
	int proto;
	int x, y, w, h;
	int tx, ty, tw, th; /* title */
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

extern const char *tags[];
extern char stext[1024];
extern int bx, by, bw, bh, bmw, mw, screen, sx, sy, sw, sh;
extern unsigned int ntags, numlockmask;
extern void (*handler[LASTEvent])(XEvent *);
extern void (*arrange)(Arg *);
extern Atom wmatom[WMLast], netatom[NetLast];
extern Bool running, issel, maximized, *seltag;
extern Client *clients, *sel, *stack;
extern Cursor cursor[CurLast];
extern DC dc;
extern Display *dpy;
extern Window root, barwin;

/* client.c */
extern void ban(Client *c);
extern void focus(Client *c);
extern Client *getclient(Window w);
extern Client *getctitle(Window w);
extern void gravitate(Client *c, Bool invert);
extern void killclient(Arg *arg);
extern void manage(Window w, XWindowAttributes *wa);
extern void resize(Client *c, Bool sizehints, Corner sticky);
extern void setsize(Client *c);
extern void settitle(Client *c);
extern void togglemax(Arg *arg);
extern void unmanage(Client *c);

/* draw.c */
extern void drawall();
extern void drawstatus();
extern void drawtitle(Client *c);
extern unsigned long getcolor(const char *colstr);
extern void setfont(const char *fontstr);
extern unsigned int textw(const char *text);

/* event.c */
extern void grabkeys();
extern void procevent();

/* main.c */
extern int getproto(Window w);
extern void quit(Arg *arg);
extern void sendevent(Window w, Atom a, long value);
extern int xerror(Display *dsply, XErrorEvent *ee);

/* tag.c */
extern void initrregs();
extern Client *getnext(Client *c);
extern Client *getprev(Client *c);
extern void settags(Client *c, Client *trans);
extern void tag(Arg *arg);
extern void toggletag(Arg *arg);

/* util.c */
extern void *emallocz(unsigned int size);
extern void eprint(const char *errstr, ...);
extern void *erealloc(void *ptr, unsigned int size);
extern void spawn(Arg *arg);

/* view.c */
extern void detach(Client *c);
extern void dofloat(Arg *arg);
extern void dotile(Arg *arg);
extern void focusnext(Arg *arg);
extern void focusprev(Arg *arg);
extern Bool isvisible(Client *c);
extern void resizecol(Arg *arg);
extern void restack();
extern void togglemode(Arg *arg);
extern void toggleview(Arg *arg);
extern void view(Arg *arg);
extern void viewall(Arg *arg);
extern void zoom(Arg *arg);
