/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <X11/Xlib.h>

/********** CUSTOMIZE **********/

#define FONT				"-*-terminus-medium-*-*-*-13-*-*-*-*-*-iso10646-*"
#define BGCOLOR				"#0a2c2d"
#define FGCOLOR				"#ddeeee"
#define BORDERCOLOR			"#176164"
#define MODKEY				 Mod1Mask /* Mod4Mask */
/*
#define BGCOLOR				"#666699"
#define FGCOLOR				"#eeeeee"
#define BORDERCOLOR			"#9999CC"
*/
#define MASTERW				52 /* percent */
#define WM_PROTOCOL_DELWIN	1

/* tags */
enum { Tscratch, Tdev, Twww, Twork, TLast };

/********** CUSTOMIZE **********/

typedef union Arg Arg;
typedef struct Client Client;
typedef enum Corner Corner;
typedef struct DC DC;
typedef struct Fnt Fnt;

union Arg {
	const char **argv;
	int i;
};

/* atoms */
enum { NetSupported, NetWMName, NetLast };
enum { WMProtocols, WMDelete, WMLast };

/* cursor */
enum { CurNormal, CurResize, CurMove, CurLast };

enum Corner { TopLeft, TopRight, BotLeft, BotRight };

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
	char *tags[TLast];
	int proto;
	int *x, *y, *w, *h; /* current geom */
	int bx, by, bw, bh; /* title bar */
	int fx, fy, fw, fh; /* floating geom */
	int tx, ty, tw, th; /* tiled geom */
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int grav;
	unsigned int border;
	long flags; 
	Bool isfloat;
	Client *next;
	Client *revert;
	Window win;
	Window title;
};

extern char *tags[TLast], stext[1024];
extern int tsel, screen, sx, sy, sw, sh, bx, by, bw, bh, mw;
extern void (*handler[LASTEvent])(XEvent *);
extern void (*arrange)(Arg *);
extern Atom wmatom[WMLast], netatom[NetLast];
extern Bool running, issel;
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
extern void lower(Client *c);
extern void manage(Window w, XWindowAttributes *wa);
extern void maximize(Arg *arg);
extern void pop(Client *c);
extern void resize(Client *c, Bool inc, Corner sticky);
extern void setgeom(Client *c);
extern void setsize(Client *c);
extern void settitle(Client *c);
extern void unmanage(Client *c);
extern void zoom(Arg *arg);

/* draw.c */
extern void drawall();
extern void drawstatus();
extern void drawtitle(Client *c);
extern unsigned long getcolor(const char *colstr);
extern void setfont(const char *fontstr);
extern unsigned int textw(char *text);

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
extern Client *getnext(Client *c, unsigned int t);
extern void heretag(Arg *arg);
extern void replacetag(Arg *arg);
extern void settags(Client *c);
extern void view(Arg *arg);

/* util.c */
extern void *emallocz(unsigned int size);
extern void eprint(const char *errstr, ...);
extern void spawn(Arg *arg);
