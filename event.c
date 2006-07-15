/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"

#include <stdlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#define ButtonMask      (ButtonPressMask | ButtonReleaseMask)
#define MouseMask       (ButtonMask | PointerMotionMask)

/********** CUSTOMIZE **********/

const char *term[] = { 
	"urxvtc", "-tr", "+sb", "-bg", "black", "-fg", "white", "-cr", "white",
	"-fn", "-*-terminus-medium-*-*-*-13-*-*-*-*-*-iso10646-*", NULL
};
const char *browse[] = { "firefox", NULL };
const char *xlock[] = { "xlock", NULL };

Key key[] = {
	/* modifier				key			function	arguments */
	{ Mod1Mask,				XK_Return,	zoom,		{ 0 } },
	{ Mod1Mask,				XK_k,		focusprev,		{ 0 } },
	{ Mod1Mask,				XK_j,		focusnext,		{ 0 } }, 
	{ Mod1Mask,				XK_m,		maximize,		{ 0 } }, 
	{ Mod1Mask,				XK_0,		view,		{ .i = Tscratch } }, 
	{ Mod1Mask,				XK_1,		view,		{ .i = Tdev } }, 
	{ Mod1Mask,				XK_2,		view,		{ .i = Twww } }, 
	{ Mod1Mask,				XK_3,		view,		{ .i = Twork } }, 
	{ Mod1Mask,				XK_space,	dotile,		{ 0 } }, 
	{ Mod1Mask|ShiftMask,	XK_space,	dofloat,	{ 0 } }, 
	{ Mod1Mask|ShiftMask,	XK_0,		replacetag,		{ .i = Tscratch } }, 
	{ Mod1Mask|ShiftMask,	XK_1,		replacetag,		{ .i = Tdev } }, 
	{ Mod1Mask|ShiftMask,	XK_2,		replacetag,		{ .i = Twww } }, 
	{ Mod1Mask|ShiftMask,	XK_3,		replacetag,		{ .i = Twork } }, 
	{ Mod1Mask|ShiftMask,	XK_c,		killclient,		{ 0 } }, 
	{ Mod1Mask|ShiftMask,	XK_q,		quit,		{ 0 } },
	{ Mod1Mask|ShiftMask,	XK_Return,	spawn,		{ .argv = term } },
	{ Mod1Mask|ShiftMask,	XK_w,		spawn,		{ .argv = browse } },
	{ Mod1Mask|ShiftMask,	XK_l,		spawn,		{ .argv = xlock } },
	{ ControlMask,			XK_0,		appendtag,	{ .i = Tscratch } }, 
	{ ControlMask,			XK_1,		appendtag,	{ .i = Tdev } }, 
	{ ControlMask,			XK_2,		appendtag,	{ .i = Twww } }, 
	{ ControlMask,			XK_3,		appendtag,	{ .i = Twork } }, 
};

/********** CUSTOMIZE **********/

/* static functions */

static void
movemouse(Client *c)
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
			XSync(dpy, False);
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
resizemouse(Client *c)
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
			XSync(dpy, False);
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
buttonpress(XEvent *e)
{
	int x;
	Arg a;
	XButtonPressedEvent *ev = &e->xbutton;
	Client *c;

	if(barwin == ev->window) {
		switch(ev->button) {
		default:
			x = 0;
			for(a.i = 0; a.i < TLast; a.i++) {
				x += textw(tags[a.i]);
				if(ev->x < x) {
					view(&a);
					break;
				}
			}
			break;
		case Button4:
			a.i = (tsel + 1 < TLast) ? tsel + 1 : 0;
			view(&a);
			break;
		case Button5:
			a.i = (tsel - 1 >= 0) ? tsel - 1 : TLast - 1;
			view(&a);
			break;
		}
	}
	else if((c = getclient(ev->window))) {
		if(arrange == dotile && !c->isfloat) {
			if((ev->state & ControlMask) && (ev->button == Button1))
				zoom(NULL);
			return;
		}
		/* floating windows */
		higher(c);
		switch(ev->button) {
		default:
			break;
		case Button1:
			movemouse(c);
			break;
		case Button2:
			lower(c);
			break;
		case Button3:
			resizemouse(c);
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
	XSync(dpy, False);
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
expose(XEvent *e)
{
	XExposeEvent *ev = &e->xexpose;
	Client *c;

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
	XKeyEvent *ev = &e->xkey;
	static unsigned int len = key ? sizeof(key) / sizeof(key[0]) : 0;
	unsigned int i;
	KeySym keysym;

	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
	for(i = 0; i < len; i++)
		if((keysym == key[i].keysym) && (key[i].mod == ev->state)) {
			if(key[i].func)
				key[i].func(&key[i].arg);
			return;
		}
}

static void
leavenotify(XEvent *e)
{
	XCrossingEvent *ev = &e->xcrossing;

	if((ev->window == root) && !ev->same_screen)
		issel = True;
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

/* extern functions */

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

void
grabkeys()
{
	static unsigned int len = key ? sizeof(key) / sizeof(key[0]) : 0;
	unsigned int i;
	KeyCode code;

	for(i = 0; i < len; i++) {
		code = XKeysymToKeycode(dpy, key[i].keysym);
		XUngrabKey(dpy, code, key[i].mod, root);
		XGrabKey(dpy, code, key[i].mod, root, True,
				GrabModeAsync, GrabModeAsync);
	}
}
