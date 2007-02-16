/* (C)opyright MMVI-MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"
#include <stdio.h>

/* static */

static Client *
nexttiled(Client *c) {
	for(; c && (c->isfloat || !isvisible(c)); c = c->next);
	return c;
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
			if(c->isbanned)
				XMoveWindow(dpy, c->win, c->x, c->y);
			c->isbanned = False;
		}
		else {
			c->isbanned = True;
			XMoveWindow(dpy, c->win, c->x + 2 * sw, c->y);
		}
	}
	if(!sel || !isvisible(sel)) {
		for(c = stack; c && !isvisible(c); c = c->snext);
		focus(c);
	}
	restack();
}

void
dotile(void) {
	unsigned int i, n, nx, ny, nw, nh, mw, mh, tw, th;
	Client *c;

	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next))
		n++;
	/* window geoms */
	mh = (n > nmaster) ? wah / nmaster : wah / (n > 0 ? n : 1);
	mw = (n > nmaster) ? (waw * master) / 1000 : waw;
	th = (n > nmaster) ? wah / (n - nmaster) : 0;
	tw = waw - mw;

	for(i = 0, c = clients; c; c = c->next)
		if(isvisible(c)) {
			if(c->isbanned)
				XMoveWindow(dpy, c->win, c->x, c->y);
			c->isbanned = False;
			if(c->isfloat)
				continue;
			c->ismax = False;
			nx = wax;
			ny = way;
			if(i < nmaster) {
				ny += i * mh;
				nw = mw - 2 * BORDERPX;
				nh = mh - 2 * BORDERPX;
			}
			else {  /* tile window */
				nx += mw;
				nw = tw - 2 * BORDERPX;
				if(th > 2 * BORDERPX) {
					ny += (i - nmaster) * th;
					nh = th - 2 * BORDERPX;
				}
				else /* fallback if th <= 2 * BORDERPX */
					nh = wah - 2 * BORDERPX;
			}
			resize(c, nx, ny, nw, nh, False);
			i++;
		}
		else {
			c->isbanned = True;
			XMoveWindow(dpy, c->win, c->x + 2 * sw, c->y);
		}
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

void
incnmaster(Arg *arg) {
	if((arrange == dofloat) || (nmaster + arg->i < 1)
	|| (wah / (nmaster + arg->i) <= 2 * BORDERPX))
		return;
	nmaster += arg->i;
	if(sel)
		arrange();
	else
		drawstatus();
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
		if(waw * (master + arg->i) / 1000 >= waw - 2 * BORDERPX
		|| waw * (master + arg->i) / 1000 <= 2 * BORDERPX)
			return;
		master += arg->i;
	}
	arrange();
}

void
restack(void) {
	Client *c;
	XEvent ev;

	drawstatus();
	if(!sel)
		return;
	if(sel->isfloat || arrange == dofloat)
		XRaiseWindow(dpy, sel->win);
	if(arrange != dofloat) {
		if(!sel->isfloat)
			XLowerWindow(dpy, sel->win);
		for(c = nexttiled(clients); c; c = nexttiled(c->next)) {
			if(c == sel)
				continue;
			XLowerWindow(dpy, c->win);
		}
	}
	XSync(dpy, False);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
togglefloat(Arg *arg) {
	if(!sel || arrange == dofloat)
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
	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next))
		n++;

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
