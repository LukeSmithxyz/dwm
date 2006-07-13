/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wm.h"

#define ButtonMask      (ButtonPressMask | ButtonReleaseMask)
#define MouseMask       (ButtonMask | PointerMotionMask)

void
mresize(Client *c)
{
	XEvent ev;
	int ocx, ocy;

	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MouseMask, GrabModeAsync, GrabModeAsync,
				None, cursor[CurResize], CurrentTime) != GrabSuccess)
		return;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w, c->h);
	for(;;) {
		XMaskEvent(dpy, MouseMask | ExposureMask, &ev);
		switch(ev.type) {
		default: break;
		case Expose:
			handler[Expose](&ev);
			break;
		case MotionNotify:
			XFlush(dpy);
			c->w = abs(ocx - ev.xmotion.x);
			c->h = abs(ocy - ev.xmotion.y);
			c->x = (ocx <= ev.xmotion.x) ? ocx : ocx - c->w;
			c->y = (ocy <= ev.xmotion.y) ? ocy : ocy - c->h;
			resize(c);
			break;
		case ButtonRelease:
			XUngrabPointer(dpy, CurrentTime);
			return;
		}
	}
}

void
mmove(Client *c)
{
	XEvent ev;
	int x1, y1, ocx, ocy, di;
	unsigned int dui;
	Window dummy;

	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MouseMask, GrabModeAsync, GrabModeAsync,
				None, cursor[CurMove], CurrentTime) != GrabSuccess)
		return;
	XQueryPointer(dpy, root, &dummy, &dummy, &x1, &y1, &di, &di, &dui);
	for(;;) {
		XMaskEvent(dpy, MouseMask | ExposureMask, &ev);
		switch (ev.type) {
		default: break;
		case Expose:
			handler[Expose](&ev);
			break;
		case MotionNotify:
			XFlush(dpy);
			c->x = ocx + (ev.xmotion.x - x1);
			c->y = ocy + (ev.xmotion.y - y1);
			resize(c);
			break;
		case ButtonRelease:
			XUngrabPointer(dpy, CurrentTime);
			return;
		}
	}
}
