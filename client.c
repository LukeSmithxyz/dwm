/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <string.h>
#include <X11/Xatom.h>

#include "util.h"
#include "wm.h"

static void
update_client_name(Client *c)
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
}

Client *
create_client(Window w, XWindowAttributes *wa)
{
	Client *c;
	XSetWindowAttributes twa;
	long msize;

	c = emallocz(sizeof(Client));
	c->win = w;
	c->r[RFloat].x = wa->x;
	c->r[RFloat].y = wa->y;
	c->r[RFloat].width = wa->width;
	c->r[RFloat].height = wa->height;
	c->border = wa->border_width;
	XSetWindowBorderWidth(dpy, c->win, 0);
	XGetTransientForHint(dpy, c->win, &c->trans);
	if(!XGetWMNormalHints(dpy, c->win, &c->size, &msize) || !c->size.flags)
		c->size.flags = PSize;
	c->fixedsize =
		(c->size.flags & PMinSize && c->size.flags & PMaxSize
		 && c->size.min_width == c->size.max_width
		 && c->size.min_height == c->size.max_height);
	update_client_name(c);
	twa.override_redirect = 1;
	twa.background_pixmap = ParentRelative;
	twa.event_mask = ExposureMask;

	c->title = XCreateWindow(dpy, root, c->r[RFloat].x, c->r[RFloat].y,
			c->r[RFloat].width, barrect.height, 0,
			DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &twa);
	XFlush(dpy);

#if 0
	for(t=&client, i=0; *t; t=&(*t)->next, i++);
	c->next = *t; /* *t == nil */
	*t = c;
#endif
	return c;
}

void
manage(Client *c)
{
	XMapRaised(dpy, c->win);
	XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
	XFlush(dpy);
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
