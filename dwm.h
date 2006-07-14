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

extern int tsel, screen, sx, sy, sw, sh, bx, by, bw, bh, mw;
extern char *tags[TLast], stext[1024];

extern DC dc;
extern Client *clients, *sel;

/* bar.c */
extern void draw_bar();
extern void barclick(XButtonPressedEvent *e);

/* client.c */
extern void manage(Window w, XWindowAttributes *wa);
extern void unmanage(Client *c);
extern Client *getclient(Window w);
extern void focus(Client *c);
extern void update_name(Client *c);
extern void draw_client(Client *c);
extern void resize(Client *c, Bool inc);
extern void update_size(Client *c);
extern Client *gettitle(Window w);
extern void craise(Client *c);
extern void lower(Client *c);
extern void ckill(Arg *arg);
extern void nextc(Arg *arg);
extern void prevc(Arg *arg);
extern void max(Arg *arg);
extern void floating(Arg *arg);
extern void tiling(Arg *arg);
extern void ttrunc(Arg *arg);
extern void tappend(Arg *arg);
extern void view(Arg *arg);
extern void zoom(Arg *arg);
extern void gravitate(Client *c, Bool invert);

/* draw.c */
extern void drawtext(const char *text, Bool border);
extern unsigned long initcolor(const char *colstr);
extern void initfont(const char *fontstr);
extern unsigned int textnw(char *text, unsigned int len);
extern unsigned int textw(char *text);
extern unsigned int texth(void);

/* dev.c */
extern void update_keys(void);
extern void keypress(XEvent *e);
extern void mresize(Client *c);
extern void mmove(Client *c);

/* main.c */
extern int error_handler(Display *dsply, XErrorEvent *e);
extern void send_message(Window w, Atom a, long value);
extern int win_proto(Window w);
extern void quit(Arg *arg);

/* util.c */
extern void error(const char *errstr, ...);
extern void *emallocz(unsigned int size);
extern void spawn(Arg *arg);
extern void swap(void **p1, void **p2);
