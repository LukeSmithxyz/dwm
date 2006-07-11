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
static void buttonpress(XEvent *e);
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
	[ButtonPress] = buttonpress,
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
discard_events(long even_mask)
{
	XEvent ev;
	unsigned int n = 0;
	while(XCheckMaskEvent(dpy, even_mask, &ev)) n++;
	return n;
}

static void
buttonpress(XEvent *e)
{
	XButtonPressedEvent *ev = &e->xbutton;
	Client *c;

	if((c = getclient(ev->window))) {
		switch(ev->button) {
		default:
			break;
		case Button1:
			mmove(c);
			break;
		case Button2:
			XLowerWindow(dpy, c->win);
			break;
		case Button3:
			mresize(c);
			break;
		}
	}
}

static void
configurerequest(XEvent *e)
{
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;
	Client *c;

	ev->value_mask &= ~CWSibling;
	if((c = getclient(ev->window))) {
		if(ev->value_mask & CWX)
			c->x = ev->x;
		if(ev->value_mask & CWY)
			c->y = ev->y;
		if(ev->value_mask & CWWidth)
			c->w = ev->width;
		if(ev->value_mask & CWHeight)
			c->h = ev->height;
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
				update_size(c);
			case XA_WM_NORMAL_HINTS:
				update_size(c);
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
