/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "config.h"
#include "draw.h"
#include "util.h"

#include <X11/Xutil.h>

/* atoms */
enum { WMState, WMProtocols, WMDelete, WMLast };
enum { NetSupported, NetWMName, NetLast };

/* cursor */
enum { CurNormal, CurResize, CurMove, CurInput, CurLast };

/* rects */
enum { RFloat, RGrid, RLast };

typedef struct Client Client;

struct Client {
	char name[256];
	char tag[256];
	int proto;
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

extern Display *dpy;
extern Window root, barwin;
extern Atom wm_atom[WMLast], net_atom[NetLast];
extern Cursor cursor[CurLast];
extern XRectangle rect, barrect;
extern Bool running;
extern Bool grid;
extern void (*handler[LASTEvent]) (XEvent *);

extern int screen, sel_screen;
extern unsigned int lock_mask, numlock_mask;
extern char *bartext, tag[256];

extern Brush brush;
extern Client *client;

/* bar.c */
extern void draw_bar();

/* client.c */
extern Client *create_client(Window w, XWindowAttributes *wa);
extern void manage(Client *c);

/* wm.c */
extern int win_proto(Window w);
