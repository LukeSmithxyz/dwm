/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <stdlib.h>
#include <string.h>
#include <X11/Xatom.h>

#include "util.h"
#include "wm.h"

#define CLIENT_MASK		(StructureNotifyMask | PropertyChangeMask | EnterWindowMask)

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
	if(c == stack)
		draw_bar();
	else
		draw_client(c);
}

void
update_size(Client *c)
{
	XSizeHints size;
	long msize;
	if(!XGetWMNormalHints(dpy, c->win, &size, &msize) || !size.flags)
		size.flags = PSize;
	c->flags = size.flags;
	c->basew = size.base_width;
	c->baseh = size.base_height;
	c->incw = size.width_inc;
	c->inch = size.height_inc;
	c->maxw = size.max_width;
	c->maxh = size.max_height;
	c->minw = size.min_width;
	c->minh = size.min_height;
}

void
focus(Client *c)
{
	Client **l;
	for(l=&stack; *l && *l != c; l=&(*l)->snext);
	eassert(*l == c);
	*l = c->snext;
	c->snext = stack;
	stack = c;
	XRaiseWindow(dpy, c->win);
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
	c->x = wa->x;
	c->y = wa->y;
	c->w = wa->width;
	c->h = wa->height;
	update_size(c);
	XSetWindowBorderWidth(dpy, c->win, 1);
	XSelectInput(dpy, c->win, CLIENT_MASK);
	XGetTransientForHint(dpy, c->win, &c->trans);
	update_name(c);
	twa.override_redirect = 1;
	twa.background_pixmap = ParentRelative;
	twa.event_mask = ExposureMask;

	c->title = XCreateWindow(dpy, root, c->x, c->y, c->w, barrect.height,
			0, DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &twa);

	for(l=&clients; *l; l=&(*l)->next);
	c->next = *l; /* *l == nil */
	*l = c;
	c->snext = stack;
	stack = c;
	XMapWindow(dpy, c->win);
	XGrabButton(dpy, Button1, Mod1Mask, c->win, False, ButtonPressMask,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button2, Mod1Mask, c->win, False, ButtonPressMask,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button3, Mod1Mask, c->win, False, ButtonPressMask,
			GrabModeAsync, GrabModeSync, None, None);
	focus(c);
}

void
resize(Client *c)
{
	XConfigureEvent e;

	XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
	e.type = ConfigureNotify;
	e.event = c->win;
	e.window = c->win;
	e.x = c->x;
	e.y = c->y;
	e.width = c->w;
	e.height = c->h;
	e.border_width = 0;
	e.above = None;
	e.override_redirect = False;
	XSelectInput(dpy, c->win, CLIENT_MASK & ~StructureNotifyMask);
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&e);
	XSelectInput(dpy, c->win, CLIENT_MASK);
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
	XUnmapWindow(dpy, c->win);
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
	discard_events(EnterWindowMask);
	if(stack)
		focus(stack);
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
	


}
