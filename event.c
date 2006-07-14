/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#include "dwm.h"

#define ButtonMask      (ButtonPressMask | ButtonReleaseMask)
#define MouseMask       (ButtonMask | PointerMotionMask)

/* local functions */
static void buttonpress(XEvent *e);
static void configurerequest(XEvent *e);
static void destroynotify(XEvent *e);
static void enternotify(XEvent *e);
static void leavenotify(XEvent *e);
static void expose(XEvent *e);
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
	[MapRequest] = maprequest,
	[PropertyNotify] = propertynotify,
	[UnmapNotify] = unmapnotify
};

static void
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
			resize(c, True);
			break;
		case ButtonRelease:
			XUngrabPointer(dpy, CurrentTime);
			return;
		}
	}
}

static void
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
			resize(c, False);
			break;
		case ButtonRelease:
			XUngrabPointer(dpy, CurrentTime);
			return;
		}
	}
}

static void
buttonpress(XEvent *e)
{
	int x;
	Arg a;
	XButtonPressedEvent *ev = &e->xbutton;
	Client *c;

	if(barwin == ev->window) {
		x = (arrange == floating) ? textw("~") : 0;
		for(a.i = 0; a.i < TLast; a.i++) {
			x += textw(tags[a.i]);
			if(ev->x < x) {
				view(&a);
				break;
			}
		}
	}
	else if((c = getclient(ev->window))) {
		if(arrange == tiling && !c->floating)
			return;
		higher(c);
		switch(ev->button) {
		default:
			break;
		case Button1:
			mmove(c);
			break;
		case Button2:
			lower(c);
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
		gravitate(c, True);
		if(ev->value_mask & CWX)
			c->x = ev->x;
		if(ev->value_mask & CWY)
			c->y = ev->y;
		if(ev->value_mask & CWWidth)
			c->w = ev->width;
		if(ev->value_mask & CWHeight)
			c->h = ev->height;
		if(ev->value_mask & CWBorderWidth)
			c->border = 1;
		gravitate(c, False);
		resize(c, True);
	}

	wc.x = ev->x;
	wc.y = ev->y;
	wc.width = ev->width;
	wc.height = ev->height;
	wc.border_width = 1;
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
	else if(ev->window == root)
		issel = True;
}

static void
leavenotify(XEvent *e)
{
	XCrossingEvent *ev = &e->xcrossing;

	if((ev->window == root) && !ev->same_screen)
		issel = True;
}

static void
expose(XEvent *e)
{
	XExposeEvent *ev = &e->xexpose;
	Client *c;

	if(ev->count == 0) {
		if(barwin == ev->window)
			drawstatus();
		else if((c = gettitle(ev->window)))
			drawtitle(c);
	}
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
	Window trans;
	Client *c;

	if(ev->state == PropertyDelete)
		return; /* ignore */

	if((c = getclient(ev->window))) {
		if(ev->atom == wm_atom[WMProtocols]) {
			c->proto = proto(c->win);
			return;
		}
		switch (ev->atom) {
			default: break;
			case XA_WM_TRANSIENT_FOR:
				XGetTransientForHint(dpy, c->win, &trans);
				if(!c->floating && (c->floating = (trans != 0)))
					arrange(NULL);
				break;
			case XA_WM_NORMAL_HINTS:
				setsize(c);
				break;
		}
		if(ev->atom == XA_WM_NAME || ev->atom == net_atom[NetWMName]) {
			settitle(c);
			drawtitle(c);
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
