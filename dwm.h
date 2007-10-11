/* See LICENSE file for copyright and license details. */
#include <X11/Xlib.h>

/* typedefs */
typedef struct Client Client;
struct Client {
	char name[256];
	int x, y, w, h;
	int rx, ry, rw, rh; /* revert geometry */
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int minax, maxax, minay, maxay;
	long flags;
	unsigned int border, oldborder;
	Bool isbanned, isfixed, ismax, isfloating, wasfloating;
	Bool *tags;
	Client *next;
	Client *prev;
	Client *snext;
	Window win;
};
