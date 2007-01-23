/* (C)opyright MMVI-MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"
#include <stdlib.h>
#include <string.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

/* static functions */

static void
detachstack(Client *c) {
	Client **tc;
	for(tc=&stack; *tc && *tc != c; tc=&(*tc)->snext);
	*tc = c->snext;
}

static void
grabbuttons(Client *c, Bool focused) {
	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);

	if(focused) {
		XGrabButton(dpy, Button1, MODKEY, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button1, MODKEY | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button1, MODKEY | numlockmask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button1, MODKEY | numlockmask | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);

		XGrabButton(dpy, Button2, MODKEY, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button2, MODKEY | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button2, MODKEY | numlockmask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button2, MODKEY | numlockmask | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);

		XGrabButton(dpy, Button3, MODKEY, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button3, MODKEY | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button3, MODKEY | numlockmask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button3, MODKEY | numlockmask | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
	}
	else
		XGrabButton(dpy, AnyButton, AnyModifier, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
}

static int
xerrordummy(Display *dsply, XErrorEvent *ee) {
	return 0;
}

/* extern functions */

void
configure(Client *c) {
	XEvent synev;

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
	XSendEvent(dpy, c->win, True, NoEventMask, &synev);
}

void
focus(Client *c) {
	if(c && !isvisible(c))
		return;

	if(sel && sel != c) {
		grabbuttons(sel, False);
		XSetWindowBorder(dpy, sel->win, dc.norm[ColBorder]);
	}
	sel = c;
	if(!issel)
		return;
	if(c) {
		detachstack(c);
		c->snext = stack;
		stack = c;
		grabbuttons(c, True);
		XSetWindowBorder(dpy, c->win, dc.sel[ColBorder]);
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
	}
	else
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
	drawstatus();
}

Client *
getclient(Window w) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(c->win == w)
			return c;
	return NULL;
}

void
killclient(Arg *arg) {
	if(!sel)
		return;
	if(sel->proto & PROTODELWIN)
		sendevent(sel->win, wmatom[WMProtocols], wmatom[WMDelete]);
	else
		XKillClient(dpy, sel->win);
}

void
manage(Window w, XWindowAttributes *wa) {
	Client *c;
	Window trans;

	c = emallocz(sizeof(Client));
	c->tags = emallocz(ntags * sizeof(Bool));
	c->win = w;
	c->x = wa->x;
	c->y = wa->y;
	c->w = wa->width;
	c->h = wa->height;
	if(c->w == sw && c->h == sh) {
		c->border = 0;
		c->x = sx;
		c->y = sy;
	}
	else {
		c->border = BORDERPX;
		if(c->x < wax)
			c->x = wax;
		if(c->y < way)
			c->y = way;
		if(c->x + c->w + 2 * c->border > wax + waw)
			c->x = wax + waw - c->w - 2 * c->border;
		if(c->y + c->h + 2 * c->border > way + wah)
			c->y = way + wah - c->h - 2 * c->border;
	}
	updatesizehints(c);
	c->proto = getproto(c->win);
	XSelectInput(dpy, c->win,
		StructureNotifyMask | PropertyChangeMask | EnterWindowMask);
	XGetTransientForHint(dpy, c->win, &trans);
	grabbuttons(c, False);
	updatetitle(c);
	settags(c, getclient(trans));
	if(!c->isfloat)
		c->isfloat = trans || c->isfixed;
	if(clients)
		clients->prev = c;
	c->next = clients;
	c->snext = stack;
	stack = clients = c;
	XMoveWindow(dpy, c->win, c->x + 2 * sw, c->y);
	XMapWindow(dpy, c->win);
	if(isvisible(c))
		focus(c);
	arrange();
}

void
resize(Client *c, Bool sizehints) {
	XWindowChanges wc;

	if(sizehints) {
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
	}
	if(c->w == sw && c->h == sh)
		c->border = 0;
	else
		c->border = BORDERPX;
	/* offscreen appearance fixes */
	if(c->x + c->w + 2 * c->border < sx)
		c->x = sx;
	if(c->y + c->h + 2 * c->border < sy)
		c->y = sy;
	if(c->x > sw)
		c->x = sw - c->w - 2 * c->border;
	if(c->y > sh)
		c->y = sh - c->h - 2 * c->border;
	wc.x = c->x;
	wc.y = c->y;
	wc.width = c->w;
	wc.height = c->h;
	wc.border_width = c->border;
	XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
	configure(c);
	XSync(dpy, False);
}

void
updatesizehints(Client *c) {
	long msize;
	XSizeHints size;

	if(!XGetWMNormalHints(dpy, c->win, &size, &msize) || !size.flags)
		size.flags = PSize;
	c->flags = size.flags;
	if(c->flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	}
	else
		c->basew = c->baseh = 0;
	if(c->flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	}
	else
		c->incw = c->inch = 0;
	if(c->flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	}
	else
		c->maxw = c->maxh = 0;
	if(c->flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	}
	else
		c->minw = c->minh = 0;
	c->isfixed = (c->maxw && c->minw && c->maxh && c->minh &&
				c->maxw == c->minw && c->maxh == c->minh);
}

void
updatetitle(Client *c) {
	char **list = NULL;
	int n;
	XTextProperty name;

	name.nitems = 0;
	c->name[0] = 0;
	XGetTextProperty(dpy, c->win, &name, netatom[NetWMName]);
	if(!name.nitems)
		XGetWMName(dpy, c->win, &name);
	if(!name.nitems)
		return;
	if(name.encoding == XA_STRING)
		strncpy(c->name, (char *)name.value, sizeof c->name);
	else {
		if(XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success
				&& n > 0 && *list)
		{
			strncpy(c->name, *list, sizeof c->name);
			XFreeStringList(list);
		}
	}
	XFree(name.value);
}

void
unmanage(Client *c) {
	Client *nc;

	/* The server grab construct avoids race conditions. */
	XGrabServer(dpy);
	XSetErrorHandler(xerrordummy);
	detach(c);
	detachstack(c);
	if(sel == c) {
		for(nc = stack; nc && !isvisible(nc); nc = nc->snext);
		focus(nc);
	}
	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
	free(c->tags);
	free(c);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XUngrabServer(dpy);
	arrange();
}
