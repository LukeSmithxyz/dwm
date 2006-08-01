/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"

#include <stdlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

/* CUSTOMIZE */

typedef struct {
	unsigned long mod;
	KeySym keysym;
	void (*func)(Arg *arg);
	Arg arg;
} Key;

const char *browse[] = { "firefox", NULL };
const char *gimp[] = { "gimp", NULL };
const char *term[] = { /*"xterm", NULL };*/
	"urxvt", "-tr", "+sb", "-bg", "black", "-fg", "white", "-cr", "white",
	"-fn", "-*-terminus-medium-*-*-*-13-*-*-*-*-*-iso10646-*", NULL
};
const char *xlock[] = { "xlock", NULL };

static Key key[] = {
	/* modifier		key		function	arguments */
	{ MODKEY,		XK_0,		view,		{ .i = Tfnord } }, 
	{ MODKEY,		XK_1,		view,		{ .i = Tdev } }, 
	{ MODKEY,		XK_2,		view,		{ .i = Tnet } }, 
	{ MODKEY,		XK_3,		view,		{ .i = Twork } }, 
	{ MODKEY,		XK_4,		view,		{ .i = Tmisc} }, 
	{ MODKEY,		XK_j,		focusnext,	{ 0 } }, 
	{ MODKEY,		XK_k,		focusprev,	{ 0 } },
	{ MODKEY,		XK_m,		togglemax,	{ 0 } }, 
	{ MODKEY,		XK_space,	togglemode,	{ 0 } }, 
	{ MODKEY,		XK_Return,	zoom,		{ 0 } },
	{ MODKEY|ControlMask,	XK_0,		appendtag,	{ .i = Tfnord } }, 
	{ MODKEY|ControlMask,	XK_1,		appendtag,	{ .i = Tdev } }, 
	{ MODKEY|ControlMask,	XK_2,		appendtag,	{ .i = Tnet } }, 
	{ MODKEY|ControlMask,	XK_3,		appendtag,	{ .i = Twork } }, 
	{ MODKEY|ControlMask,	XK_4,		appendtag,	{ .i = Tmisc } }, 
	{ MODKEY|ShiftMask,	XK_0,		replacetag,	{ .i = Tfnord } }, 
	{ MODKEY|ShiftMask,	XK_1,		replacetag,	{ .i = Tdev } }, 
	{ MODKEY|ShiftMask,	XK_2,		replacetag,	{ .i = Tnet } }, 
	{ MODKEY|ShiftMask,	XK_3,		replacetag,	{ .i = Twork } }, 
	{ MODKEY|ShiftMask,	XK_4,		replacetag,	{ .i = Tmisc } }, 
	{ MODKEY|ShiftMask,	XK_c,		killclient,	{ 0 } }, 
	{ MODKEY|ShiftMask,	XK_q,		quit,		{ 0 } },
	{ MODKEY|ShiftMask,	XK_Return,	spawn,		{ .argv = term } },
	{ MODKEY|ShiftMask,	XK_g,		spawn,		{ .argv = gimp } },
	{ MODKEY|ShiftMask,	XK_l,		spawn,		{ .argv = xlock } },
	{ MODKEY|ShiftMask,	XK_w,		spawn,		{ .argv = browse } },
};

/* END CUSTOMIZE */

/* static */

static void
movemouse(Client *c)
{
	int x1, y1, ocx, ocy, di;
	unsigned int dui;
	Window dummy;
	XEvent ev;

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
	Corner sticky;
	XEvent ev;

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
		focus(c);
		switch(ev->button) {
		default:
			break;
		case Button1:
			if(!c->ismax && (arrange == dofloat || c->isfloat)) {
				higher(c);
				movemouse(c);
			}
			break;
		case Button2:
			lower(c);
			break;
		case Button3:
			if(!c->ismax && (arrange == dofloat || c->isfloat)) {
				higher(c);
				resizemouse(c);
			}
			break;
		}
	}
}

static void
configurerequest(XEvent *e)
{
	Client *c;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;

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
		resize(c, True, TopLeft);
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
	Client *c;
	XCrossingEvent *ev = &e->xcrossing;

	if(ev->detail == NotifyInferior)
		return;

	if((c = getclient(ev->window)))
		focus(c);
	else if(ev->window == root)
		issel = True;
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

	for(i = 0; i < len; i++) {
		code = XKeysymToKeycode(dpy, key[i].keysym);
		XUngrabKey(dpy, code, key[i].mod, root);
		XGrabKey(dpy, code, key[i].mod, root, True,
				GrabModeAsync, GrabModeAsync);
	}
}
