/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xatom.h>

#include "util.h"
#include "wm.h"

void
max(void *aux)
{
	if(!stack)
		return;
	stack->x = sx;
	stack->y = bh;
	stack->w = sw - 2 * stack->border;
	stack->h = sh - bh - 2 * stack->border;
	resize(stack);
	discard_events(EnterWindowMask);
}

void
arrange(void *aux)
{
	Client *c;
	int n, cols, rows, gw, gh, i, j;
    float rt, fd;

	if(!clients)
		return;
	for(n = 0, c = clients; c; c = c->next, n++);
	rt = sqrt(n);
	if(modff(rt, &fd) < 0.5)
		rows = floor(rt);
	else
		rows = ceil(rt);
	if(rows * rows < n)
		cols = rows + 1;
	else
		cols = rows;

	gw = (sw - 2 * c->border)  / cols;
	gh = (sh - bh - 2 * c->border) / rows;

	for(i = j = 0, c = clients; c; c = c->next) {
		c->x = i * gw;
		c->y = j * gh + bh;
		c->w = gw;
		c->h = gh;
		resize(c);
		if(++i == cols) {
			j++;
			i = 0;
		}
	}
	discard_events(EnterWindowMask);
}

void
sel(void *aux)
{
	const char *arg = aux;
	Client *c = NULL;

	if(!arg || !stack)
		return;
	if(!strncmp(arg, "next", 5))
		c = stack->snext ? stack->snext : stack;
	else if(!strncmp(arg, "prev", 5))
		for(c = stack; c && c->snext; c = c->snext);
	if(!c)
		c = stack;
	raise(c);
	focus(c);
}

void
kill(void *aux)
{
	Client *c = stack;

	if(!c)
		return;
	if(c->proto & WM_PROTOCOL_DELWIN)
		send_message(c->win, wm_atom[WMProtocols], wm_atom[WMDelete]);
	else
		XKillClient(dpy, c->win);
}

static void
resize_title(Client *c)
{
	c->tw = textw(&brush.font, c->name) + bh;
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
raise(Client *c)
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
	Client **l, *old;

	old = stack;
	for(l = &stack; *l && *l != c; l = &(*l)->snext);
	eassert(*l == c);
	*l = c->snext;
	c->snext = stack;
	stack = c;
	if(old && old != c) {
		XMapWindow(dpy, old->title);
		draw_client(old);
	}
	XUnmapWindow(dpy, c->title);
	draw_client(c);
	XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
	XFlush(dpy);
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
	if(c->y < bh)
		c->ty = c->y += bh;
	c->tw = c->w = wa->width;
	c->h = wa->height;
	c->th = bh;
	c->border = 1;
	update_size(c);
	XSetWindowBorderWidth(dpy, c->win, 1);
	XSetWindowBorder(dpy, c->win, brush.border);
	XSelectInput(dpy, c->win,
			StructureNotifyMask | PropertyChangeMask | EnterWindowMask);
	XGetTransientForHint(dpy, c->win, &c->trans);
	twa.override_redirect = 1;
	twa.background_pixmap = ParentRelative;
	twa.event_mask = ExposureMask;

	c->title = XCreateWindow(dpy, root, c->tx, c->ty, c->tw, c->th,
			0, DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &twa);
	update_name(c);

	for(l=&clients; *l; l=&(*l)->next);
	c->next = *l; /* *l == nil */
	*l = c;
	c->snext = stack;
	stack = c;
	XMapRaised(dpy, c->win);
	XMapRaised(dpy, c->title);
	XGrabButton(dpy, Button1, Mod1Mask, c->win, False, ButtonPressMask,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button2, Mod1Mask, c->win, False, ButtonPressMask,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button3, Mod1Mask, c->win, False, ButtonPressMask,
			GrabModeAsync, GrabModeSync, None, None);
	resize(c);
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
dummy_error_handler(Display *dpy, XErrorEvent *error)
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

	for(l=&clients; *l && *l != c; l=&(*l)->next);
	eassert(*l == c);
	*l = c->next;
	for(l=&stack; *l && *l != c; l=&(*l)->snext);
	eassert(*l == c);
	*l = c->snext;
	free(c);

	XFlush(dpy);
	XSetErrorHandler(error_handler);
	XUngrabServer(dpy);
	if(stack)
		focus(stack);
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
	if(c == stack) {
		draw_bar();
		return;
	}

	brush.x = brush.y = 0;
	brush.w = c->tw;
	brush.h = c->th;

	draw(dpy, &brush, True, c->name);
	XCopyArea(dpy, brush.drawable, c->title, brush.gc,
			0, 0, c->tw, c->th, 0, 0);
	XFlush(dpy);
}
