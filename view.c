/* (C)opyright MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"

/* static */

static Client *
nexttiled(Client *c) {
	for(c = getnext(c); c && c->isfloat; c = getnext(c->next));
	return c;
}

static void
togglemax(Client *c) {
	XEvent ev;
		
	if(c->isfixed)
		return;

	if((c->ismax = !c->ismax)) {
		c->rx = c->x; c->x = wax;
		c->ry = c->y; c->y = way;
		c->rw = c->w; c->w = waw - 2 * BORDERPX;
		c->rh = c->h; c->h = wah - 2 * BORDERPX;
	}
	else {
		c->x = c->rx;
		c->y = c->ry;
		c->w = c->rw;
		c->h = c->rh;
	}
	resize(c, True, TopLeft);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

/* extern */

void (*arrange)(void) = DEFMODE;

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
dofloat(void) {
	Client *c;

	for(c = clients; c; c = c->next) {
		if(isvisible(c)) {
			resize(c, True, TopLeft);
		}
		else
			ban(c);
	}
	if(!sel || !isvisible(sel)) {
		for(c = stack; c && !isvisible(c); c = c->snext);
		focus(c);
	}
	restack();
}

void
dotile(void) {
	unsigned int i, n, mpw, th;
	Client *c;

	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next))
		n++;
	mpw = (waw * master) / 1000;

	for(i = 0, c = clients; c; c = c->next)
		if(isvisible(c)) {
			if(c->isfloat) {
				resize(c, True, TopLeft);
				continue;
			}
			c->ismax = False;
			c->x = wax;
			c->y = way;
			if(n == 1) { /* only 1 window */
				c->w = waw - 2 * BORDERPX;
				c->h = wah - 2 * BORDERPX;
			}
			else if(i == 0) { /* master window */
				c->w = mpw - 2 * BORDERPX;
				c->h = wah - 2 * BORDERPX;
				th = wah / (n - 1);
			}
			else {  /* tile window */
				c->x += mpw;
				c->w = (waw - mpw) - 2 * BORDERPX;
				if(th > bh) {
					c->y += (i - 1) * th;
					c->h = th - 2 * BORDERPX;
				}
				else /* fallback if th < bh */
					c->h = wah - 2 * BORDERPX;
			}
			resize(c, False, TopLeft);
			i++;
		}
		else
			ban(c);

	if(!sel || !isvisible(sel)) {
		for(c = stack; c && !isvisible(c); c = c->snext);
		focus(c);
	}
	restack();
}

void
focusnext(Arg *arg) {
	Client *c;
   
	if(!sel)
		return;
	if(!(c = getnext(sel->next)))
		c = getnext(clients);
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
	if(!(c = getprev(sel->prev))) {
		for(c = clients; c && c->next; c = c->next);
		c = getprev(c);
	}
	if(c) {
		focus(c);
		restack();
	}
}

Bool
isvisible(Client *c) {
	unsigned int i;

	for(i = 0; i < ntags; i++)
		if(c->tags[i] && seltag[i])
			return True;
	return False;
}

void
resizemaster(Arg *arg) {
	if(arg->i == 0)
		master = MASTER;
	else {
		if(master + arg->i > 950 || master + arg->i < 50)
			return;
		master += arg->i;
	}
	arrange();
}

void
restack(void) {
	Client *c;
	XEvent ev;

	if(!sel) {
		drawstatus();
		return;
	}
	if(sel->isfloat || arrange == dofloat) {
		XRaiseWindow(dpy, sel->win);
		XRaiseWindow(dpy, sel->twin);
	}
	if(arrange != dofloat) {
		if(!sel->isfloat) {
			XLowerWindow(dpy, sel->twin);
			XLowerWindow(dpy, sel->win);
		}
		for(c = nexttiled(clients); c; c = nexttiled(c->next)) {
			if(c == sel)
				continue;
			XLowerWindow(dpy, c->twin);
			XLowerWindow(dpy, c->win);
		}
	}
	drawall();
	XSync(dpy, False);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
togglefloat(Arg *arg) {
	if (!sel || arrange == dofloat)
		return;
	sel->isfloat = !sel->isfloat;
	arrange();
}

void
togglemode(Arg *arg) {
	arrange = (arrange == dofloat) ? dotile : dofloat;
	if(sel)
		arrange();
	else
		drawstatus();
}

void
toggleview(Arg *arg) {
	unsigned int i;

	seltag[arg->i] = !seltag[arg->i];
	for(i = 0; i < ntags && !seltag[i]; i++);
	if(i == ntags)
		seltag[arg->i] = True; /* cannot toggle last view */
	arrange();
}

void
view(Arg *arg) {
	unsigned int i;

	for(i = 0; i < ntags; i++)
		seltag[i] = (arg->i == -1) ? True : False;
	if(arg->i >= 0 && arg->i < ntags)
		seltag[arg->i] = True;
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
	for(n = 0, c = clients; c; c = c->next)
		if(isvisible(c) && !c->isfloat)
			n++;
	if(n < 2 || (arrange == dofloat))
		return;
	if((c = sel) == nexttiled(clients))
		if(!(c = nexttiled(c->next)))
			return;
	detach(c);
	if(clients)
		clients->prev = c;
	c->next = clients;
	clients = c;
	focus(c);
	arrange();
}
