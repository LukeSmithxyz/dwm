/* (C)opyright MMVI-MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"
#include <stdlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

/* static */

typedef struct {
	unsigned long mod;
	KeySym keysym;
	void (*func)(Arg *arg);
	Arg arg;
} Key;

KEYS

#define CLEANMASK(mask) (mask & ~(numlockmask | LockMask))
#define MOUSEMASK		(BUTTONMASK | PointerMotionMask)

static void
movemouse(Client *c) {
	int x1, y1, ocx, ocy, di;
	unsigned int dui;
	Window dummy;
	XEvent ev;

	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
			None, cursor[CurMove], CurrentTime) != GrabSuccess)
		return;
	c->ismax = False;
	XQueryPointer(dpy, root, &dummy, &dummy, &x1, &y1, &di, &di, &dui);
	for(;;) {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ButtonRelease:
			resize(c, True);
			XUngrabPointer(dpy, CurrentTime);
			return;
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			XSync(dpy, False);
			c->x = ocx + (ev.xmotion.x - x1);
			c->y = ocy + (ev.xmotion.y - y1);
			if(abs(wax + c->x) < SNAP)
				c->x = wax;
			else if(abs((wax + waw) - (c->x + c->w + 2 * c->border)) < SNAP)
				c->x = wax + waw - c->w - 2 * c->border;
			if(abs(way - c->y) < SNAP)
				c->y = way;
			else if(abs((way + wah) - (c->y + c->h + 2 * c->border)) < SNAP)
				c->y = way + wah - c->h - 2 * c->border;
			resize(c, False);
			break;
		}
	}
}

static void
resizemouse(Client *c) {
	int ocx, ocy;
	int nw, nh;
	XEvent ev;

	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
			None, cursor[CurResize], CurrentTime) != GrabSuccess)
		return;
	c->ismax = False;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->border - 1, c->h + c->border - 1);
	for(;;) {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask , &ev);
		switch(ev.type) {
		case ButtonRelease:
			resize(c, True);
			XUngrabPointer(dpy, CurrentTime);
			return;
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			XSync(dpy, False);
			nw = ev.xmotion.x - ocx - 2 * c->border + 1;
			c->w = nw > 0 ? nw : 1;
			nh = ev.xmotion.y - ocy - 2 * c->border + 1;
			c->h = nh > 0 ? nh : 1;
			resize(c, True);
			break;
		}
	}
}

static void
buttonpress(XEvent *e) {
	int x;
	Arg a;
	Client *c;
	XButtonPressedEvent *ev = &e->xbutton;

	if(barwin == ev->window) {
		x = 0;
		for(a.i = 0; a.i < ntags; a.i++) {
			x += textw(tags[a.i]);
			if(ev->x < x) {
				if(ev->button == Button1) {
					if(ev->state & MODKEY)
						tag(&a);
					else
						view(&a);
				}
				else if(ev->button == Button3) {
					if(ev->state & MODKEY)
						toggletag(&a);
					else
						toggleview(&a);
				}
				return;
			}
		}
		if(ev->x < x + bmw)
			switch(ev->button) {
			case Button1:
				togglemode(NULL);
				break;
			case Button4:
				a.i = 1;
				incnmaster(&a);
				break;
			case Button5:
				a.i = -1;
				incnmaster(&a);
				break;
			}
	}
	else if((c = getclient(ev->window))) {
		focus(c);
		if(CLEANMASK(ev->state) != MODKEY)
			return;
		if(ev->button == Button1 && (arrange == dofloat || c->isfloat)) {
			restack();
			movemouse(c);
		}
		else if(ev->button == Button2)
			zoom(NULL);
		else if(ev->button == Button3 && (arrange == dofloat || c->isfloat) &&
				!c->isfixed) {
			restack();
			resizemouse(c);
		}
	}
}

static void
configurerequest(XEvent *e) {
	unsigned long newmask;
	Client *c;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;

	if((c = getclient(ev->window))) {
		c->ismax = False;
		if(ev->value_mask & CWX)
			c->x = ev->x;
		if(ev->value_mask & CWY)
			c->y = ev->y;
		if(ev->value_mask & CWWidth)
			c->w = ev->width;
		if(ev->value_mask & CWHeight)
			c->h = ev->height;
		if(ev->value_mask & CWBorderWidth)
			c->border = ev->border_width;
		wc.x = c->x;
		wc.y = c->y;
		wc.width = c->w;
		wc.height = c->h;
		newmask = ev->value_mask & (~(CWSibling | CWStackMode | CWBorderWidth));
		if(newmask)
			XConfigureWindow(dpy, c->win, newmask, &wc);
		else
			configure(c);
		XSync(dpy, False);
		if(c->isfloat) {
			resize(c, False);
			if(!isvisible(c))
				XMoveWindow(dpy, c->win, c->x + 2 * sw, c->y);
		}
		else
			arrange();
	}
	else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
		XSync(dpy, False);
	}
}

static void
destroynotify(XEvent *e) {
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if((c = getclient(ev->window)))
		unmanage(c);
}

static void
enternotify(XEvent *e) {
	Client *c;
	XCrossingEvent *ev = &e->xcrossing;

	if(ev->mode != NotifyNormal || ev->detail == NotifyInferior)
		return;
	if((c = getclient(ev->window)) && isvisible(c))
		focus(c);
	else if(ev->window == root) {
		activescreen = True;
		for(c = stack; c && !isvisible(c); c = c->snext);
		focus(c);
	}
}

static void
expose(XEvent *e) {
	XExposeEvent *ev = &e->xexpose;

	if(ev->count == 0) {
		if(barwin == ev->window)
			drawstatus();
	}
}

static void
keypress(XEvent *e) {
	static unsigned int len = sizeof key / sizeof key[0];
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ev = &e->xkey;

	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
	for(i = 0; i < len; i++) {
		if(keysym == key[i].keysym
			&& CLEANMASK(key[i].mod) == CLEANMASK(ev->state))
		{
			if(key[i].func)
				key[i].func(&key[i].arg);
		}
	}
}

static void
leavenotify(XEvent *e) {
	XCrossingEvent *ev = &e->xcrossing;

	if((ev->window == root) && !ev->same_screen) {
		activescreen = False;
		focus(NULL);
	}
}

static void
mappingnotify(XEvent *e) {
	XMappingEvent *ev = &e->xmapping;

	XRefreshKeyboardMapping(ev);
	if(ev->request == MappingKeyboard)
		grabkeys();
}

static void
maprequest(XEvent *e) {
	static XWindowAttributes wa;
	XMapRequestEvent *ev = &e->xmaprequest;

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
propertynotify(XEvent *e) {
	Client *c;
	Window trans;
	XPropertyEvent *ev = &e->xproperty;

	if(ev->state == PropertyDelete)
		return; /* ignore */
	if((c = getclient(ev->window))) {
		if(ev->atom == wmatom[WMProtocols]) {
			c->proto = getproto(c->win);
			return;
		}
		switch (ev->atom) {
			default: break;
			case XA_WM_TRANSIENT_FOR:
				XGetTransientForHint(dpy, c->win, &trans);
				if(!c->isfloat && (c->isfloat = (trans != 0)))
					arrange();
				break;
			case XA_WM_NORMAL_HINTS:
				updatesizehints(c);
				break;
		}
		if(ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
			updatetitle(c);
			if(c == sel)
				drawstatus();
		}
	}
}

static void
unmapnotify(XEvent *e) {
	Client *c;
	XUnmapEvent *ev = &e->xunmap;

	if((c = getclient(ev->window)))
		unmanage(c);
}

/* extern */

void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress] = buttonpress,
	[ConfigureRequest] = configurerequest,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = enternotify,
	[LeaveNotify] = leavenotify,
	[Expose] = expose,
	[KeyPress] = keypress,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[PropertyNotify] = propertynotify,
	[UnmapNotify] = unmapnotify
};

void
grabkeys(void) {
	static unsigned int len = sizeof key / sizeof key[0];
	unsigned int i;
	KeyCode code;

	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	for(i = 0; i < len; i++) {
		code = XKeysymToKeycode(dpy, key[i].keysym);
		XGrabKey(dpy, code, key[i].mod, root, True,
				GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, code, key[i].mod | LockMask, root, True,
				GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, code, key[i].mod | numlockmask, root, True,
				GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, code, key[i].mod | numlockmask | LockMask, root, True,
				GrabModeAsync, GrabModeAsync);
	}
}

void
procevent(void) {
	XEvent ev;

	while(XPending(dpy)) {
		XNextEvent(dpy, &ev);
		if(handler[ev.type])
			(handler[ev.type])(&ev); /* call handler */
	}
}
