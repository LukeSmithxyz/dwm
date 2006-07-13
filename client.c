/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "dwm.h"

static void (*arrange)(Arg *) = floating;

static void
center(Client *c)
{
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w / 2, c->h / 2);
}

static Client *
next(Client *c)
{
	for(; c && !c->tags[tsel]; c = c->next);
	return c;
}

void
zoom(Arg *arg)
{
	Client **l;

	if(!sel)
		return;

	for(l = &clients; *l && *l != sel; l = &(*l)->next);
	*l = sel->next;

	sel->next = clients; /* pop */
	clients = sel;
	arrange(NULL);
	center(sel);
	focus(sel);
}

void
max(Arg *arg)
{
	if(!sel)
		return;
	sel->x = sx;
	sel->y = sy;
	sel->w = sw - 2 * sel->border;
	sel->h = sh - 2 * sel->border;
	craise(sel);
	resize(sel);
	discard_events(EnterWindowMask);
}

void
view(Arg *arg)
{
	tsel = arg->i;
	arrange(NULL);
}

void
tag(Arg *arg)
{
	int i, n;
	if(!sel)
		return;

	if(arg->i == tsel) {
		for(n = i = 0; i < TLast; i++)
			if(sel->tags[i])
				n++;
		if(n < 2)
			return;
	}

	if(sel->tags[arg->i])
		sel->tags[arg->i] = NULL; /* toggle tag */
	else
		sel->tags[arg->i] = tags[arg->i];
	arrange(NULL);
}

static void
ban_client(Client *c)
{
	XMoveWindow(dpy, c->win, c->x + 2 * sw, c->y);
	XMoveWindow(dpy, c->title, c->tx + 2 * sw, c->ty);
}

void
floating(Arg *arg)
{
	Client *c;

	arrange = floating;
	for(c = clients; c; c = c->next) {
		if(c->tags[tsel])
			resize(c);
		else
			ban_client(c);
	}
	if(sel && !sel->tags[tsel]) {
		if((sel = next(clients))) {
			craise(sel);
			focus(sel);
		}
	}
	discard_events(EnterWindowMask);
}

void
tiling(Arg *arg)
{
	Client *c;
	int n, cols, rows, gw, gh, i, j;
    float rt, fd;

	arrange = tiling;
	for(n = 0, c = clients; c; c = next(c->next), n++);
	if(n) {
		rt = sqrt(n);
		if(modff(rt, &fd) < 0.5)
			rows = floor(rt);
		else
			rows = ceil(rt);
		if(rows * rows < n)
			cols = rows + 1;
		else
			cols = rows;

		gw = (sw - 2)  / cols;
		gh = (sh - 2) / rows;
	}
	else
		cols = rows = gw = gh = 0;

	for(i = j = 0, c = clients; c; c = c->next) {
		if(c->tags[tsel]) {
			c->x = i * gw;
			c->y = j * gh;
			c->w = gw;
			c->h = gh;
			resize(c);
			if(++i == cols) {
				j++;
				i = 0;
			}
		}
		else
			ban_client(c);
	}
	if(sel && !sel->tags[tsel]) {
		if((sel = next(clients))) {
			craise(sel);
			focus(sel);
		}
	}
	discard_events(EnterWindowMask);
}

void
prevc(Arg *arg)
{
	Client *c;

	if(!sel)
		return;

	if((c = sel->revert && sel->revert->tags[tsel] ? sel->revert : NULL)) {
		craise(c);
		center(c);
		focus(c);
	}
}

void
nextc(Arg *arg)
{
	Client *c;
   
	if(!sel)
		return;

	if(!(c = next(sel->next)))
		c = next(clients);
	if(c) {
		craise(c);
		center(c);
		c->revert = sel;
		focus(c);
	}
}

void
ckill(Arg *arg)
{
	if(!sel)
		return;
	if(sel->proto & WM_PROTOCOL_DELWIN)
		send_message(sel->win, wm_atom[WMProtocols], wm_atom[WMDelete]);
	else
		XKillClient(dpy, sel->win);
}

static void
resize_title(Client *c)
{
	int i;

	c->tw = 0;
	for(i = 0; i < TLast; i++)
		if(c->tags[i])
			c->tw += textw(c->tags[i]) + dc.font.height;
	c->tw += textw(c->name) + dc.font.height;
	if(c->tw > c->w)
		c->tw = c->w + 2;
	c->tx = c->x + c->w - c->tw + 2;
	c->ty = c->y;
	XMoveResizeWindow(dpy, c->title, c->tx, c->ty, c->tw, c->th);
}

void
update_name(Client *c)
{
	XTextProperty name;
	int n;
	char **list = NULL;

	name.nitems = 0;
	c->name[0] = 0;
	XGetTextProperty(dpy, c->win, &name, net_atom[NetWMName]);
	if(!name.nitems)
		XGetWMName(dpy, c->win, &name);
	if(!name.nitems)
		return;
	if(name.encoding == XA_STRING)
		strncpy(c->name, (char *)name.value, sizeof(c->name));
	else {
		if(XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success
				&& n > 0 && *list)
		{
			strncpy(c->name, *list, sizeof(c->name));
			XFreeStringList(list);
		}
	}
	XFree(name.value);
	resize_title(c);
}

void
update_size(Client *c)
{
	XSizeHints size;
	long msize;
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
	if(c->flags & PWinGravity)
		c->grav = size.win_gravity;
	else
		c->grav = NorthWestGravity;
}

void
craise(Client *c)
{
	XRaiseWindow(dpy, c->win);
	XRaiseWindow(dpy, c->title);
}

void
lower(Client *c)
{
	XLowerWindow(dpy, c->title);
	XLowerWindow(dpy, c->win);
}

void
focus(Client *c)
{
	if(sel && sel != c) {
		XSetWindowBorder(dpy, sel->win, dc.bg);
		XMapWindow(dpy, sel->title);
		draw_client(sel);
	}
	sel = c;
	XUnmapWindow(dpy, c->title);
	XSetWindowBorder(dpy, c->win, dc.fg);
	draw_client(c);
	XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
	XFlush(dpy);
	discard_events(EnterWindowMask);
}

void
manage(Window w, XWindowAttributes *wa)
{
	Client *c, **l;
	XSetWindowAttributes twa;

	c = emallocz(sizeof(Client));
	c->win = w;
	c->tx = c->x = wa->x;
	c->ty = c->y = wa->y;
	c->tw = c->w = wa->width;
	c->h = wa->height;
	c->th = th;
	c->border = 1;
	c->proto = win_proto(c->win);
	update_size(c);
	XSelectInput(dpy, c->win,
			StructureNotifyMask | PropertyChangeMask | EnterWindowMask);
	XGetTransientForHint(dpy, c->win, &c->trans);
	twa.override_redirect = 1;
	twa.background_pixmap = ParentRelative;
	twa.event_mask = ExposureMask;

	c->tags[tsel] = tags[tsel];
	c->title = XCreateWindow(dpy, root, c->tx, c->ty, c->tw, c->th,
			0, DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &twa);

	update_name(c);

	for(l = &clients; *l; l = &(*l)->next);
	c->next = *l; /* *l == nil */
	*l = c;

	XSetWindowBorderWidth(dpy, c->win, 1);
	XMapRaised(dpy, c->win);
	XMapRaised(dpy, c->title);
	XGrabButton(dpy, Button1, Mod1Mask, c->win, False, ButtonPressMask,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button2, Mod1Mask, c->win, False, ButtonPressMask,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button3, Mod1Mask, c->win, False, ButtonPressMask,
			GrabModeAsync, GrabModeSync, None, None);
	arrange(NULL);
	center(c);
	focus(c);
}

void
gravitate(Client *c, Bool invert)
{
	int dx = 0, dy = 0;

	switch(c->grav) {
	case StaticGravity:
	case NorthWestGravity:
	case NorthGravity:
	case NorthEastGravity:
		dy = c->border;
		break;
	case EastGravity:
	case CenterGravity:
	case WestGravity:
		dy = -(c->h / 2) + c->border;
		break;
	case SouthEastGravity:
	case SouthGravity:
	case SouthWestGravity:
		dy = -c->h;
		break;
	default:
		break;
	}

	switch (c->grav) {
	case StaticGravity:
	case NorthWestGravity:
	case WestGravity:
	case SouthWestGravity:
		dx = c->border;
		break;
	case NorthGravity:
	case CenterGravity:
	case SouthGravity:
		dx = -(c->w / 2) + c->border;
		break;
	case NorthEastGravity:
	case EastGravity:
	case SouthEastGravity:
		dx = -(c->w + c->border);
		break;
	default:
		break;
	}

	if(invert) {
		dx = -dx;
		dy = -dy;
	}
	c->x += dx;
	c->y += dy;
}


void
resize(Client *c)
{
	XConfigureEvent e;

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
	resize_title(c);
	XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
	e.type = ConfigureNotify;
	e.event = c->win;
	e.window = c->win;
	e.x = c->x;
	e.y = c->y;
	e.width = c->w;
	e.height = c->h;
	e.border_width = c->border;
	e.above = None;
	e.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&e);
	XFlush(dpy);
}

static int
dummy_error_handler(Display *dsply, XErrorEvent *err)
{
	return 0;
}

void
unmanage(Client *c)
{
	Client **l;

	XGrabServer(dpy);
	XSetErrorHandler(dummy_error_handler);

	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
	XDestroyWindow(dpy, c->title);

	for(l = &clients; *l && *l != c; l = &(*l)->next);
	*l = c->next;
	for(l = &clients; *l; l = &(*l)->next)
		if((*l)->revert == c)
			(*l)->revert = NULL;
	if(sel == c)
		sel = sel->revert ? sel->revert : clients;

	free(c);

	XFlush(dpy);
	XSetErrorHandler(error_handler);
	XUngrabServer(dpy);
	arrange(NULL);
	if(sel)
		focus(sel);
}

Client *
gettitle(Window w)
{
	Client *c;
	for(c = clients; c; c = c->next)
		if(c->title == w)
			return c;
	return NULL;
}

Client *
getclient(Window w)
{
	Client *c;
	for(c = clients; c; c = c->next)
		if(c->win == w)
			return c;
	return NULL;
}

void
draw_client(Client *c)
{
	int i;
	if(c == sel)
		return;

	dc.x = dc.y = 0;
	dc.h = c->th;

	dc.w = 0;
	for(i = 0; i < TLast; i++) {
		if(c->tags[i]) {
			dc.x += dc.w;
			dc.w = textw(c->tags[i]) + dc.font.height;
			draw(True, c->tags[i]);
		}
	}
	dc.x += dc.w;
	dc.w = textw(c->name) + dc.font.height;
	draw(True, c->name);
	XCopyArea(dpy, dc.drawable, c->title, dc.gc,
			0, 0, c->tw, c->th, 0, 0);
	XFlush(dpy);
}
