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

typedef union Arg Arg;
typedef struct Client Client;
typedef struct DC DC;
typedef struct Fnt Fnt;

union Arg {
	const char *cmd;
	int i;
};

/* atoms */
enum { NetSupported, NetWMName, NetLast };
enum { WMProtocols, WMDelete, WMLast };

/* cursor */
enum { CurNormal, CurResize, CurMove, CurLast };

/* windowcorners */
typedef enum { TopLeft, TopRight, BotLeft, BotRight } Corner;

struct Fnt {
	int ascent;
	int descent;
	int height;
	XFontSet set;
	XFontStruct *xfont;
};

struct DC { /* draw context */
	int x, y, w, h;
	unsigned long bg;
	unsigned long fg;
	unsigned long border;
	Drawable drawable;
	Fnt font;
	GC gc;
};

struct Client {
	char name[256];
	int proto;
	int x, y, w, h;
	int tx, ty, tw, th; /* title */
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int grav;
	long flags; 
	unsigned int border;
	Bool isfloat;
	Bool ismax;
	Bool *tags;
	Client *next;
	Client *prev;
	Window win;
	Window title;
};

extern const char *tags[];
extern char stext[1024];
extern int screen, sx, sy, sw, sh, bx, by, bw, bh, mw;
extern unsigned int ntags;
extern void (*handler[LASTEvent])(XEvent *);
extern void (*arrange)(Arg *);
extern Atom wmatom[WMLast], netatom[NetLast];
extern Bool running, issel, *seltag;
extern Client *clients, *sel;
extern Cursor cursor[CurLast];
extern DC dc;
extern Display *dpy;
extern Window root, barwin;

/* client.c */
extern void ban(Client *c);
extern void focus(Client *c);
extern void focusnext(Arg *arg);
extern void focusprev(Arg *arg);
extern Client *getclient(Window w);
extern Client *getctitle(Window w);
extern void gravitate(Client *c, Bool invert);
extern void higher(Client *c);
extern void killclient(Arg *arg);
extern void manage(Window w, XWindowAttributes *wa);
extern void resize(Client *c, Bool sizehints, Corner sticky);
extern void setsize(Client *c);
extern void settitle(Client *c);
extern void togglemax(Arg *arg);
extern void unmanage(Client *c);
extern void zoom(Arg *arg);

/* draw.c */
extern void drawall();
extern void drawstatus();
extern void drawtitle(Client *c);
extern unsigned long getcolor(const char *colstr);
extern void setfont(const char *fontstr);
extern unsigned int textw(const char *text);

/* event.c */
extern void grabkeys();

/* main.c */
extern int getproto(Window w);
extern void quit(Arg *arg);
extern void sendevent(Window w, Atom a, long value);
extern int xerror(Display *dsply, XErrorEvent *ee);

/* tag.c */
extern void appendtag(Arg *arg);
extern void dofloat(Arg *arg);
extern void dotile(Arg *arg);
extern void initrregs();
extern Bool isvisible(Client *c);
extern Client *getnext(Client *c);
extern Client *getprev(Client *c);
extern void replacetag(Arg *arg);
extern void settags(Client *c);
extern void togglemode(Arg *arg);
extern void view(Arg *arg);
extern void toggleview(Arg *arg);

/* util.c */
extern void *emallocz(unsigned int size);
extern void eprint(const char *errstr, ...);
extern void spawn(Arg *arg);
