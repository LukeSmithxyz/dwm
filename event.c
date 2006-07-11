/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#include "wm.h"

/* local functions */
static void configurerequest(XEvent *e);
static void destroynotify(XEvent *e);
static void enternotify(XEvent *e);
static void leavenotify(XEvent *e);
static void expose(XEvent *e);
static void keymapnotify(XEvent *e);
static void maprequest(XEvent *e);
static void propertynotify(XEvent *e);
static void unmapnotify(XEvent *e);

void (*handler[LASTEvent]) (XEvent *) = {
	[ConfigureRequest] = configurerequest,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = enternotify,
	[LeaveNotify] = leavenotify,
	[Expose] = expose,
	[KeyPress] = keypress,
	[KeymapNotify] = keymapnotify,
	[MapRequest] = maprequest,
	[PropertyNotify] = propertynotify,
	[UnmapNotify] = unmapnotify
};

unsigned int
flush_events(long even_mask)
{
	XEvent ev;
	unsigned int n = 0;
	while(XCheckMaskEvent(dpy, even_mask, &ev)) n++;
	return n;
}

static void
configurerequest(XEvent *e)
{
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;
	Client *c;

	c = getclient(ev->window);
	ev->value_mask &= ~CWSibling;
	if(c) {
		if(ev->value_mask & CWX)
			c->r[RFloat].x = ev->x;
		if(ev->value_mask & CWY)
			c->r[RFloat].y = ev->y;
		if(ev->value_mask & CWWidth)
			c->r[RFloat].width = ev->width;
		if(ev->value_mask & CWHeight)
			c->r[RFloat].height = ev->height;
		if(ev->value_mask & CWBorderWidth)
			c->border = ev->border_width;
	}

	wc.x = ev->x;
	wc.y = ev->y;
	wc.width = ev->width;
	wc.height = ev->height;
	wc.border_width = 0;
	wc.sibling = None;
	wc.stack_mode = Above;
	ev->value_mask &= ~CWStackMode;
	ev->value_mask |= CWBorderWidth;
	XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
	XFlush(dpy);
}

static void
destroynotify(XEvent *e)
{
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if((c = getclient(ev->window)))
		unmanage(c);
}

static void
enternotify(XEvent *e)
{
	XCrossingEvent *ev = &e->xcrossing;
	Client *c;

	if(ev->mode != NotifyNormal || ev->detail == NotifyInferior)
		return;

	if((c = getclient(ev->window)))
		focus(c);
	else if(ev->window == root) {
		sel_screen = True;
		/*draw_frames();*/
	}
}

static void
leavenotify(XEvent *e)
{
	XCrossingEvent *ev = &e->xcrossing;

	if((ev->window == root) && !ev->same_screen) {
		sel_screen = True;
		/*draw_frames();*/
	}
}

static void
expose(XEvent *e)
{
	XExposeEvent *ev = &e->xexpose;

	if(ev->count == 0) {
		if(ev->window == barwin)
			draw_bar();
	}
}

static void
keymapnotify(XEvent *e)
{
	update_keys();
}

static void
maprequest(XEvent *e)
{
	XMapRequestEvent *ev = &e->xmaprequest;
	static XWindowAttributes wa;

	if(!XGetWindowAttributes(dpy, ev->window, &wa))
		return;

	if(wa.override_redirect) {
		XSelectInput(dpy, ev->window,
				(StructureNotifyMask | PropertyChangeMask));
		return;
	}

	if(!getclient(ev->window))
		manage(ev->window, &wa);
}

static void
propertynotify(XEvent *e)
{
	XPropertyEvent *ev = &e->xproperty;
	long msize;
	Client *c;

	if(ev->state == PropertyDelete)
		return; /* ignore */

	if(ev->atom == wm_atom[WMProtocols]) {
		c->proto = win_proto(c->win);
		return;
	}
	if((c = getclient(ev->window))) {
		switch (ev->atom) {
			default: break;
			case XA_WM_TRANSIENT_FOR:
				XGetTransientForHint(dpy, c->win, &c->trans);
				break;
			case XA_WM_NORMAL_HINTS:
				if(!XGetWMNormalHints(dpy, c->win, &c->size, &msize)
						|| !c->size.flags)
					c->size.flags = PSize;
				if(c->size.flags & PMinSize && c->size.flags & PMaxSize
						&& c->size.min_width == c->size.max_width
						&& c->size.min_height == c->size.max_height)
					c->fixedsize = True;
				else
					c->fixedsize = False;
				break;
		}
		if(ev->atom == XA_WM_NAME || ev->atom == net_atom[NetWMName]) {
			update_name(c);
		}
	}
}

static void
unmapnotify(XEvent *e)
{
	Client *c;
	XUnmapEvent *ev = &e->xunmap;

	if((c = getclient(ev->window)))
		unmanage(c);
}
