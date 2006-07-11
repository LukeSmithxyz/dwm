/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * (C)opyright MMVI Kris Maglione <fbsdaemon@gmail.com>
 * See LICENSE file for license details.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wm.h"

#define ButtonMask      (ButtonPressMask | ButtonReleaseMask)
#define MouseMask       (ButtonMask | PointerMotionMask)

static void
mmatch(Client *c, int x1, int y1, int x2, int y2)
{
	c->w = abs(x1 - x2);
	c->h = abs(y1 - y2);
	if(c->incw)
		c->w -= (c->w - c->basew) % c->incw;
	if(c->inch)
		c->h -= (c->h - c->baseh) % c->inch;
	if(c->minw && c->w < c->minw)
		c->w = c->minw;
	if(c->minh && c->h < c->minh)
		c->h = c->minh;
	if(c->maxw && c->w > c->maxw)
		c->w = c->maxw;
	if(c->maxh && c->h > c->maxh)
		c->h = c->maxh;
	c->x = (x1 <= x2) ? x1 : x1 - c->w;
	c->y = (y1 <= y2) ? y1 : y1 - c->h;
}

void
mresize(Client *c)
{
	XEvent ev;
	int old_cx, old_cy;

	old_cx = c->x;
	old_cy = c->y;
	if(XGrabPointer(dpy, root, False, MouseMask, GrabModeAsync, GrabModeAsync,
				None, cursor[CurResize], CurrentTime) != GrabSuccess)
		return;
	XGrabServer(dpy);
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w, c->h);
	for(;;) {
		XMaskEvent(dpy, MouseMask, &ev);
		switch(ev.type) {
		default: break;
		case MotionNotify:
			XUngrabServer(dpy);
			mmatch(c, old_cx, old_cy, ev.xmotion.x, ev.xmotion.y);
			XResizeWindow(dpy, c->win, c->w, c->h);
			XGrabServer(dpy);
			break;
		case ButtonRelease:
			resize(c);
			XUngrabServer(dpy);
			XUngrabPointer(dpy, CurrentTime);
			return;
		}
	}
}

void
mmove(Client *c)
{
	XEvent ev;
	int x1, y1, old_cx, old_cy, di;
	unsigned int dui;
	Window dummy;

	old_cx = c->x;
	old_cy = c->y;
	if(XGrabPointer(dpy, root, False, MouseMask, GrabModeAsync, GrabModeAsync,
				None, cursor[CurMove], CurrentTime) != GrabSuccess)
		return;
	XQueryPointer(dpy, root, &dummy, &dummy, &x1, &y1, &di, &di, &dui);
	XGrabServer(dpy);
	for(;;) {
		XMaskEvent(dpy, MouseMask, &ev);
		switch (ev.type) {
		default: break;
		case MotionNotify:
			XUngrabServer(dpy);
			c->x = old_cx + (ev.xmotion.x - x1);
			c->y = old_cy + (ev.xmotion.y - y1);
			XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
			XGrabServer(dpy);
			break;
		case ButtonRelease:
			resize(c);
			XUngrabServer(dpy);
			XUngrabPointer(dpy, CurrentTime);
			return;
		}
	}
}
