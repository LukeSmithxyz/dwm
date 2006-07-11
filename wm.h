/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "config.h"
#include "draw.h"
#include "util.h"

#include <X11/Xutil.h>

#define WM_PROTOCOL_DELWIN 1

/* atoms */
enum { WMProtocols, WMDelete, WMLast };
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
	int proto;
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
	void (*func)(void *aux);
	void *aux;
};

extern Display *dpy;
extern Window root, barwin;
extern Atom wm_atom[WMLast], net_atom[NetLast];
extern Cursor cursor[CurLast];
extern XRectangle rect, barrect;
extern Bool running, sel_screen, grid;
extern void (*handler[LASTEvent]) (XEvent *);

extern int screen;
extern char *bartext, tag[256];

extern Brush brush;
extern Client *clients, *stack;

/* bar.c */
extern void draw_bar();

/* cmd.c */
extern void run(void *aux);
extern void quit(void *aux);
extern void kill(void *aux);

/* client.c */
extern void manage(Window w, XWindowAttributes *wa);
extern void unmanage(Client *c);
extern Client *getclient(Window w);
extern void focus(Client *c);
extern void update_name(Client *c);

/* event.c */
extern unsigned int flush_events(long even_mask);

/* key.c */
extern void update_keys();
extern void keypress(XEvent *e);

/* wm.c */
extern int error_handler(Display *dpy, XErrorEvent *error);
extern void send_message(Window w, Atom a, long value);
extern int win_proto(Window w);
