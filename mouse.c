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
	c->r[RFloat].width = abs(x1 - x2);
	c->r[RFloat].height = abs(y1 - y2);
	c->r[RFloat].width -=
		(c->r[RFloat].width - c->size.base_width) % c->size.width_inc;
	c->r[RFloat].height -=
		(c->r[RFloat].height - c->size.base_height) % c->size.height_inc;
	if(c->size.min_width && c->r[RFloat].width < c->size.min_width)
		c->r[RFloat].width = c->size.min_width;
	if(c->size.min_height && c->r[RFloat].height < c->size.min_height)
		c->r[RFloat].height = c->size.min_height;
	if(c->size.max_width && c->r[RFloat].width > c->size.max_width)
		c->r[RFloat].width = c->size.max_width;
	if(c->size.max_height && c->r[RFloat].height > c->size.max_height)
		c->r[RFloat].height = c->size.max_height;
	c->r[RFloat].x = (x1 <= x2) ? x1 : x1 - c->r[RFloat].width;
	c->r[RFloat].y = (y1 <= y2) ? y1 : y1 - c->r[RFloat].height;
}

void
mresize(Client *c)
{
	XEvent ev;
	int old_cx, old_cy;

	old_cx = c->r[RFloat].x;
	old_cy = c->r[RFloat].y;
	if(XGrabPointer(dpy, c->win, False, MouseMask, GrabModeAsync, GrabModeAsync,
				None, cursor[CurResize], CurrentTime) != GrabSuccess)
		return;
	XGrabServer(dpy);
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0,
			c->r[RFloat].width, c->r[RFloat].height);
	for(;;) {
		XMaskEvent(dpy, MouseMask, &ev);
		switch(ev.type) {
		default: break;
		case MotionNotify:
			XUngrabServer(dpy);
			mmatch(c, old_cx, old_cy, ev.xmotion.x, ev.xmotion.y);
			resize(c);
			XGrabServer(dpy);
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
	int x1, y1, old_cx, old_cy, di;
	unsigned int dui;
	Window dummy;

	old_cx = c->r[RFloat].x;
	old_cy = c->r[RFloat].y;
	if(XGrabPointer(dpy, c->win, False, MouseMask, GrabModeAsync, GrabModeAsync,
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
			c->r[RFloat].x = old_cx + (ev.xmotion.x - x1);
			c->r[RFloat].y = old_cy + (ev.xmotion.y - y1);
			resize(c);
			XGrabServer(dpy);
			break;
		case ButtonRelease:
			XUngrabServer(dpy);
			XUngrabPointer(dpy, CurrentTime);
			return;
		}
	}
}
