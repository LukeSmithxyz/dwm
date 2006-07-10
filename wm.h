/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>

/* WM atoms */
enum { WMState, WMProtocols, WMDelete, WMLast };

/* NET atoms */
enum { NetSupported, NetWMName, NetLast };

/* Cursor */
enum { CurNormal, CurResize, CurMove, CurInput, CurLast };

/* Rects */
enum { RFloat, RGrid, RLast };

typedef struct Client Client;
typedef struct Tag Tag;

struct Client {
	Tag *tag;
	char name[256];
	int proto;
	Window win;
	Window trans;
	Window title;
	GC gc;
	XSizeHints size;
	XRectangle r[RLast];
	Client *next;
	Client *tnext;
	Client *tprev;
};

struct Tag {
	char name[256];
	Client *clients;
	Client *sel;
	XRectangle r;
};

extern Display *dpy;
extern Window root;
extern XRectangle rect;
extern int screen, sel_screen;
extern unsigned int kmask, numlock_mask;
extern Atom wm_atom[WMLast];
extern Atom net_atom[NetLast];
extern Cursor cursor[CurLast];
extern Pixmap pmap;

/* wm.c */
extern void error(char *errstr, ...);
