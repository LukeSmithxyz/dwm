/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "config.h"
#include "draw.h"
#include "util.h"

#include <X11/Xutil.h>

/* atoms */
enum { NetSupported, NetWMName, NetLast };

/* cursor */
enum { CurNormal, CurResize, CurMove, CurInput, CurLast };

/* rects */
enum { RFloat, RGrid, RLast };

typedef struct Client Client;
typedef struct Key Key;

struct Client {
	char name[256];
	char tag[256];
	unsigned int border;
	Bool fixedsize;
	Window win;
	Window trans;
	Window title;
	XSizeHints size;
	XRectangle r[RLast];
	Client *next;
	Client *snext;
};

struct Key {
	unsigned long mod;
	KeySym keysym;
	void (*func)(char *arg);
	char *arg;
};

extern Display *dpy;
extern Window root, barwin;
extern Atom net_atom[NetLast];
extern Cursor cursor[CurLast];
extern XRectangle rect, barrect;
extern Bool running;
extern Bool grid;
extern void (*handler[LASTEvent]) (XEvent *);

extern int screen, sel_screen;
extern char *bartext, tag[256];

extern Brush brush;
extern Client *clients;

/* bar.c */
extern void draw_bar();

/* cmd.c */
extern void run(char *arg);
extern void quit(char *arg);

/* client.c */
extern Client *create_client(Window w, XWindowAttributes *wa);
extern void manage(Client *c);
extern Client * getclient(Window w);

/* key.c */
extern void update_keys();
extern void keypress(XEvent *e);

/* wm.c */
