/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "config.h"
#include "draw.h"
#include "util.h"

#include <X11/Xutil.h>

#define WM_PROTOCOL_DELWIN 1

typedef struct Client Client;
typedef struct Key Key;

/* atoms */
enum { WMProtocols, WMDelete, WMLast };
enum { NetSupported, NetWMName, NetLast };

/* cursor */
enum { CurNormal, CurResize, CurMove, CurInput, CurLast };

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
	Window win;
	Window trans;
	Window title;
	Client *next;
	Client *snext;
};

struct Key {
	unsigned long mod;
	KeySym keysym;
	void (*func)(void *aux);
	void *aux;
};

extern Display *dpy;
extern Window root, barwin;
extern Atom wm_atom[WMLast], net_atom[NetLast];
extern Cursor cursor[CurLast];
extern Bool running, issel;
extern void (*handler[LASTEvent]) (XEvent *);
extern void (*arrange)(void *aux);

extern int tsel, screen, sx, sy, sw, sh, bx, by, bw, bh;
extern char stext[1024], *tags[TLast];

extern Brush brush;
extern Client *clients, *stack;

/* bar.c */
extern void draw_bar();

/* client.c */
extern void manage(Window w, XWindowAttributes *wa);
extern void unmanage(Client *c);
extern Client *getclient(Window w);
extern void focus(Client *c);
extern void update_name(Client *c);
extern void draw_client(Client *c);
extern void resize(Client *c);
extern void update_size(Client *c);
extern Client *gettitle(Window w);
extern void raise(Client *c);
extern void lower(Client *c);
extern void kill(void *aux);
extern void sel(void *aux);
extern void max(void *aux);
extern void floating(void *aux);
extern void grid(void *aux);
extern void gravitate(Client *c, Bool invert);

/* event.c */
extern void discard_events(long even_mask);

/* key.c */
extern void update_keys();
extern void keypress(XEvent *e);

/* mouse.c */
extern void mresize(Client *c);
extern void mmove(Client *c);

/* wm.c */
extern int error_handler(Display *dpy, XErrorEvent *error);
extern void send_message(Window w, Atom a, long value);
extern int win_proto(Window w);
extern void run(void *aux);
extern void quit(void *aux);
