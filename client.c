/* (C)opyright MMVI-MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"
#include <stdlib.h>
#include <string.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

/* static */

static void
attachstack(Client *c) {
	c->snext = stack;
	stack = c;
}

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

static Bool
isprotodel(Client *c) {
	int i, n;
	Atom *protocols;
	Bool ret = False;

	if(XGetWMProtocols(dpy, c->win, &protocols, &n)) {
		for(i = 0; !ret && i < n; i++)
			if(protocols[i] == wmatom[WMDelete])
				ret = True;
		XFree(protocols);
	}
	return ret;
}

static void
setclientstate(Client *c, long state) {
	long data[] = {state, None};
	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
			PropModeReplace, (unsigned char *)data, 2);
}

static void
togglemax(Client *c) {
	XEvent ev;

	if(c->isfixed)
		return;
	if((c->ismax = !c->ismax)) {
		c->rx = c->x;
		c->ry = c->y;
		c->rw = c->w;
		c->rh = c->h;
		resize(c, wax, way, waw - 2 * BORDERPX, wah - 2 * BORDERPX, True);
	}
	else
		resize(c, c->rx, c->ry, c->rw, c->rh, True);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

static int
xerrordummy(Display *dsply, XErrorEvent *ee) {
	return 0;
}

/* extern */

void
attach(Client *c) {
	if(clients)
		clients->prev = c;
	c->next = clients;
	clients = c;
}

void
configure(Client *c) {
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = c->border;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void
detach(Client *c) {
	if(c->prev)
		c->prev->next = c->next;
	if(c->next)
		c->next->prev = c->prev;
	if(c == clients)
		clients = c->next;
	c->next = c->prev = NULL;
}

void
focus(Client *c) {
	if(c && !isvisible(c))
		return;
	if(sel && sel != c) {
		grabbuttons(sel, False);
		XSetWindowBorder(dpy, sel->win, dc.norm[ColBorder]);
	}
	if(c) {
		detachstack(c);
		attachstack(c);
		grabbuttons(c, True);
	}
	sel = c;
	drawstatus();
	if(!selscreen)
		return;
	if(c) {
		XSetWindowBorder(dpy, c->win, dc.sel[ColBorder]);
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
	}
	else
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
}

void
focusnext(Arg *arg) {
	Client *c;
   
	if(!sel)
		return;
	for(c = sel->next; c && !isvisible(c); c = c->next);
	if(!c)
		for(c = clients; c && !isvisible(c); c = c->next);
	if(c) {
		focus(c);
		restack();
	}
}

void
focusprev(Arg *arg) {
	Client *c;

	if(!sel)
		return;
	for(c = sel->prev; c && !isvisible(c); c = c->prev);
	if(!c) {
		for(c = clients; c && c->next; c = c->next);
		for(; c && !isvisible(c); c = c->prev);
	}
	if(c) {
		focus(c);
		restack();
	}
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
	if(isprotodel(sel))
		sendevent(sel->win, wmatom[WMProtocols], wmatom[WMDelete]);
	else
		XKillClient(dpy, sel->win);
}

void
manage(Window w, XWindowAttributes *wa) {
	Client *c, *t;
	Window trans;
	XWindowChanges wc;

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
		if(c->x + c->w + 2 * c->border > wax + waw)
			c->x = wax + waw - c->w - 2 * c->border;
		if(c->y + c->h + 2 * c->border > way + wah)
			c->y = way + wah - c->h - 2 * c->border;
		if(c->x < wax)
			c->x = wax;
		if(c->y < way)
			c->y = way;
	}
	updatesizehints(c);
	XSelectInput(dpy, c->win,
		StructureNotifyMask | PropertyChangeMask | EnterWindowMask);
	XGetTransientForHint(dpy, c->win, &trans);
	grabbuttons(c, False);
	wc.border_width = c->border;
	XConfigureWindow(dpy, c->win, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, c->win, dc.norm[ColBorder]);
	configure(c); /* propagates border_width, if size doesn't change */
	updatetitle(c);
	t = getclient(trans);
	settags(c, t);
	if(!c->isfloat)
		c->isfloat = (t != 0) || c->isfixed;
	attach(c);
	attachstack(c);
	c->isbanned = True;
	XMoveWindow(dpy, c->win, c->x + 2 * sw, c->y);
	XMapWindow(dpy, c->win);
	setclientstate(c, NormalState);
	if(isvisible(c))
		focus(c);
	arrange();
}

Client *
nexttiled(Client *c) {
	for(; c && (c->isfloat || !isvisible(c)); c = c->next);
	return c;
}

void
resize(Client *c, int x, int y, int w, int h, Bool sizehints) {
	float actual, dx, dy, max, min;
	XWindowChanges wc;

	if(w <= 0 || h <= 0)
		return;
	if(sizehints) {
		if(c->minw && w < c->minw)
			w = c->minw;
		if(c->minh && h < c->minh)
			h = c->minh;
		if(c->maxw && w > c->maxw)
			w = c->maxw;
		if(c->maxh && h > c->maxh)
			h = c->maxh;
		/* inspired by algorithm from fluxbox */
		if(c->minay > 0 && c->maxay && (h - c->baseh) > 0) {
			dx = (float)(w - c->basew);
			dy = (float)(h - c->baseh);
			min = (float)(c->minax) / (float)(c->minay);
			max = (float)(c->maxax) / (float)(c->maxay);
			actual = dx / dy;
			if(max > 0 && min > 0 && actual > 0) {
				if(actual < min) {
					dy = (dx * min + dy) / (min * min + 1);
					dx = dy * min;
					w = (int)dx + c->basew;
					h = (int)dy + c->baseh;
				}
				else if(actual > max) {
					dy = (dx * min + dy) / (max * max + 1);
					dx = dy * min;
					w = (int)dx + c->basew;
					h = (int)dy + c->baseh;
				}
			}
		}
		if(c->incw)
			w -= (w - c->basew) % c->incw;
		if(c->inch)
			h -= (h - c->baseh) % c->inch;
	}
	if(w == sw && h == sh)
		c->border = 0;
	else
		c->border = BORDERPX;
	/* offscreen appearance fixes */
	if(x > sw)
		x = sw - w - 2 * c->border;
	if(y > sh)
		y = sh - h - 2 * c->border;
	if(x + w + 2 * c->border < sx)
		x = sx;
	if(y + h + 2 * c->border < sy)
		y = sy;
	if(c->x != x || c->y != y || c->w != w || c->h != h) {
		c->x = wc.x = x;
		c->y = wc.y = y;
		c->w = wc.width = w;
		c->h = wc.height = h;
		wc.border_width = c->border;
		XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
		configure(c);
		XSync(dpy, False);
	}
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
	if(c->flags & PAspect) {
		c->minax = size.min_aspect.x;
		c->minay = size.min_aspect.y;
		c->maxax = size.max_aspect.x;
		c->maxay = size.max_aspect.y;
	}
	else
		c->minax = c->minay = c->maxax = c->maxay = 0;
	c->isfixed = (c->maxw && c->minw && c->maxh && c->minh
			&& c->maxw == c->minw && c->maxh == c->minh);
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
	setclientstate(c, WithdrawnState);
	free(c->tags);
	free(c);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XUngrabServer(dpy);
	arrange();
}

void
zoom(Arg *arg) {
	unsigned int n;
	Client *c;

	if(!sel)
		return;
	if(sel->isfloat || (arrange == dofloat)) {
		togglemax(sel);
		return;
	}
	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next))
		n++;

	if((c = sel) == nexttiled(clients))
		if(!(c = nexttiled(c->next)))
			return;
	detach(c);
	attach(c);
	focus(c);
	arrange();
}
