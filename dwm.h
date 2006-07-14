/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <X11/Xlib.h>

/********** CUSTOMIZE **********/

#define FONT				"-*-terminus-medium-*-*-*-13-*-*-*-*-*-iso10646-*"
#define BGCOLOR				"#666699"
#define FGCOLOR				"#eeeeee"
#define BORDERCOLOR			"#9999CC"
#define MASTERW				52 /* percent */
#define WM_PROTOCOL_DELWIN	1

/* tags */
enum { Tscratch, Tdev, Twww, Twork, TLast };

/********** CUSTOMIZE **********/

typedef union Arg Arg;
typedef struct DC DC;
typedef struct Client Client;
typedef struct Fnt Fnt;
typedef struct Key Key;
typedef struct Rule Rule;

union Arg {
	const char **argv;
	int i;
};

/* atoms */
enum { WMProtocols, WMDelete, WMLast };
enum { NetSupported, NetWMName, NetLast };

/* cursor */
enum { CurNormal, CurResize, CurMove, CurInput, CurLast };

struct Fnt {
	XFontStruct *xfont;
	XFontSet set;
	int ascent;
	int descent;
	int height;
};

struct DC { /* draw context */
	GC gc;
	Drawable drawable;
	int x, y, w, h;
	Fnt font;
	unsigned long bg;
	unsigned long fg;
	unsigned long border;
};

struct Client {
	char name[256];
	char *tags[TLast];
	int proto;
	int x, y, w, h;
	int tx, ty, tw, th;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int grav;
	unsigned int border;
	long flags; 
	Bool floating;
	Window win;
	Window title;
	Client *next;
	Client *revert;
};

struct Rule {
	const char *class;
	const char *instance;
	char *tags[TLast];
	Bool floating;
};

struct Key {
	unsigned long mod;
	KeySym keysym;
	void (*func)(Arg *arg);
	Arg arg;
};

extern Display *dpy;
extern Window root, barwin;
extern Atom wm_atom[WMLast], net_atom[NetLast];
extern Cursor cursor[CurLast];
extern Bool running, issel;
extern void (*handler[LASTEvent])(XEvent *);
extern void (*arrange)(Arg *);
extern Key key[];

extern int tsel, screen, sx, sy, sw, sh, bx, by, bw, bh, mw;
extern char *tags[TLast], stext[1024];

extern DC dc;
extern Client *clients, *sel;

/* client.c */
extern void manage(Window w, XWindowAttributes *wa);
extern void unmanage(Client *c);
extern Client *getclient(Window w);
extern void focus(Client *c);
extern void settitle(Client *c);
extern void resize(Client *c, Bool inc);
extern void setsize(Client *c);
extern Client *gettitle(Window w);
extern void higher(Client *c);
extern void lower(Client *c);
extern void gravitate(Client *c, Bool invert);
extern void ban(Client *c);
extern Client *getnext(Client *c);

/* draw.c */
extern void drawstatus();
extern void drawtitle(Client *c);
extern void drawtext(const char *text, Bool invert, Bool border);
extern unsigned long getcolor(const char *colstr);
extern void setfont(const char *fontstr);
extern unsigned int textnw(char *text, unsigned int len);
extern unsigned int textw(char *text);
extern unsigned int texth(void);

/* key.c */
extern void grabkeys();
extern void keypress(XEvent *e);

/* main.c */
extern int xerror(Display *dsply, XErrorEvent *e);
extern void sendevent(Window w, Atom a, long value);
extern int proto(Window w);
extern void quit(Arg *arg);

/* screen.c */
extern void floating(Arg *arg);
extern void tiling(Arg *arg);
extern void view(Arg *arg);

/* util.c */
extern void error(const char *errstr, ...);
extern void *emallocz(unsigned int size);
extern void spawn(Arg *arg);
