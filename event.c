/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
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

static void
movemouse(Client *c)
{
	int x1, y1, ocx, ocy, di;
	unsigned int dui;
	Window dummy;
	XEvent ev;

	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
			None, cursor[CurMove], CurrentTime) != GrabSuccess)
		return;
	XQueryPointer(dpy, root, &dummy, &dummy, &x1, &y1, &di, &di, &dui);
	for(;;) {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask, &ev);
		switch (ev.type) {
		default: break;
		case Expose:
			handler[Expose](&ev);
			break;
		case MotionNotify:
			XSync(dpy, False);
			c->x = ocx + (ev.xmotion.x - x1);
			c->y = ocy + (ev.xmotion.y - y1);
			resize(c, False, TopLeft);
			break;
		case ButtonRelease:
			XUngrabPointer(dpy, CurrentTime);
			return;
		}
	}
}

static void
resizemouse(Client *c)
{
	int ocx, ocy;
	int nw, nh;
	Corner sticky;
	XEvent ev;

	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
				None, cursor[CurResize], CurrentTime) != GrabSuccess)
		return;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w, c->h);
	for(;;) {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask, &ev);
		switch(ev.type) {
		default: break;
		case Expose:
			handler[Expose](&ev);
			break;
		case MotionNotify:
			XSync(dpy, False);
			if((nw = abs(ocx - ev.xmotion.x)))
				c->w = abs(ocx - ev.xmotion.x);
			if((nh = abs(ocy - ev.xmotion.y)))
				c->h = abs(ocy - ev.xmotion.y);
			c->x = (ocx <= ev.xmotion.x) ? ocx : ocx - c->w;
			c->y = (ocy <= ev.xmotion.y) ? ocy : ocy - c->h;
			if(ocx <= ev.xmotion.x)
				sticky = (ocy <= ev.xmotion.y) ? TopLeft : BotLeft;
			else
				sticky = (ocy <= ev.xmotion.y) ? TopRight : BotRight;
			resize(c, True, sticky);
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
	Client *c;
	XButtonPressedEvent *ev = &e->xbutton;

	if(barwin == ev->window) {
		x = 0;
		for(a.i = 0; a.i < ntags; a.i++) {
			x += textw(tags[a.i]);
			if(ev->x < x) {
				if(ev->button == Button1)
					view(&a);
				else if(ev->button == Button3)
					toggleview(&a);
				return;
			}
		}
	}
	else if((c = getclient(ev->window))) {
		focus(c);
		if(CLEANMASK(ev->state) == 0)
			return;
		switch(ev->button) {
		default:
			break;
		case Button1:
			if(!c->ismax && (arrange == dofloat || c->isfloat)) {
				restack(c);
				movemouse(c);
			}
			break;
		case Button2:
			zoom(NULL);
			break;
		case Button3:
			if(!c->ismax && (arrange == dofloat || c->isfloat)) {
				restack(c);
				resizemouse(c);
			}
			break;
		}
	}
}

static void
configurerequest(XEvent *e)
{
	unsigned long newmask;
	Client *c;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XEvent synev;
	XWindowChanges wc;

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
			c->border = ev->border_width;
		gravitate(c, False);
		wc.x = c->x;
		wc.y = c->y;
		wc.width = c->w;
		wc.height = c->h;
		newmask = ev->value_mask & (~(CWSibling | CWStackMode | CWBorderWidth));
		if(newmask)
			XConfigureWindow(dpy, c->win, newmask, &wc);
		else {
			synev.type = ConfigureNotify;
			synev.xconfigure.display = dpy;
			synev.xconfigure.event = c->win;
			synev.xconfigure.window = c->win;
			synev.xconfigure.x = c->x;
			synev.xconfigure.y = c->y;
			synev.xconfigure.width = c->w;
			synev.xconfigure.height = c->h;
			synev.xconfigure.border_width = c->border;
			synev.xconfigure.above = None;
			/* Send synthetic ConfigureNotify */
			XSendEvent(dpy, c->win, True, NoEventMask, &synev);
		}
		XSync(dpy, False);
		if(c->isfloat)
			resize(c, False, TopLeft);
		else
			arrange(NULL);
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
	Client *c;
	XCrossingEvent *ev = &e->xcrossing;

	if(ev->mode != NotifyNormal || ev->detail == NotifyInferior)
		return;

	if((c = getclient(ev->window)) || (c = getctitle(ev->window)))
		focus(c);
	else if(ev->window == root) {
		issel = True;
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		drawall();
	}
}

static void
expose(XEvent *e)
{
	Client *c;
	XExposeEvent *ev = &e->xexpose;

	if(ev->count == 0) {
		if(barwin == ev->window)
			drawstatus();
		else if((c = getctitle(ev->window)))
			drawtitle(c);
	}
}

static void
keypress(XEvent *e)
{
	static unsigned int len = sizeof(key) / sizeof(key[0]);
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ev = &e->xkey;

	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
	for(i = 0; i < len; i++) {
		if(keysym == key[i].keysym &&
				CLEANMASK(key[i].mod) == CLEANMASK(ev->state))
		{
			if(key[i].func)
				key[i].func(&key[i].arg);
			return;
		}
	}
}

static void
leavenotify(XEvent *e)
{
	XCrossingEvent *ev = &e->xcrossing;

	if((ev->window == root) && !ev->same_screen) {
		issel = False;
		drawall();
	}
}

static void
mappingnotify(XEvent *e)
{
	XMappingEvent *ev = &e->xmapping;

	XRefreshKeyboardMapping(ev);
	if(ev->request == MappingKeyboard)
		grabkeys();
}

static void
maprequest(XEvent *e)
{
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
propertynotify(XEvent *e)
{
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
					arrange(NULL);
				break;
			case XA_WM_NORMAL_HINTS:
				setsize(c);
				break;
		}
		if(ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
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
grabkeys()
{
	static unsigned int len = sizeof(key) / sizeof(key[0]);
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
procevent()
{
	XEvent ev;

	while(XPending(dpy)) {
		XNextEvent(dpy, &ev);
		if(handler[ev.type])
			(handler[ev.type])(&ev); /* call handler */
	}
}

