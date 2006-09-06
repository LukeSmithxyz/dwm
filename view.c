/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"
#include <stdio.h>

/* static */

static Client *
minclient()
{
	Client *c, *min;

	for(min = c = clients; c; c = c->next)
		if(c->weight < min->weight)
			min = c;
	return min;
}


static void
reorder()
{
	Client *c, *newclients, *tail;

	newclients = tail = NULL;
	while((c = minclient())) {
		detach(c);
		if(tail) {
			c->prev = tail;
			tail->next = c;
			tail = c;
		}
		else
			tail = newclients = c;
	}
	clients = newclients;
}

static Client *
nexttiled(Client *c)
{
	for(c = getnext(c->next); c && c->isfloat; c = getnext(c->next));
	return c;
}

/* extern */

void (*arrange)(Arg *) = DEFMODE;

void
detach(Client *c)
{
	if(c->prev)
		c->prev->next = c->next;
	if(c->next)
		c->next->prev = c->prev;
	if(c == clients)
		clients = c->next;
	c->next = c->prev = NULL;
}

void
dofloat(Arg *arg)
{
	Client *c;

	maximized = False;

	for(c = clients; c; c = c->next) {
		if(isvisible(c)) {
			resize(c, True, TopLeft);
		}
		else
			ban(c);
	}
	if(!sel || !isvisible(sel))
		focus(getnext(clients));
	restack();
}

void
dotile(Arg *arg)
{
	int h, i, n, w;
	Client *c;

	maximized = False;

	w = sw - mw;
	for(n = 0, c = clients; c; c = c->next)
		if(isvisible(c) && !c->isfloat)
			n++;

	if(n > 1)
		h = (sh - bh) / (n - 1);
	else
		h = sh - bh;

	for(i = 0, c = clients; c; c = c->next) {
		if(isvisible(c)) {
			if(c->isfloat) {
				resize(c, True, TopLeft);
				continue;
			}
			if(n == 1) {
				c->x = sx;
				c->y = sy + bh;
				c->w = sw - 2;
				c->h = sh - 2 - bh;
			}
			else if(i == 0) {
				c->x = sx;
				c->y = sy + bh;
				c->w = mw - 2;
				c->h = sh - 2 - bh;
			}
			else if(h > bh) {
				c->x = sx + mw;
				c->y = sy + (i - 1) * h + bh;
				c->w = w - 2;
				if(i + 1 == n)
					c->h = sh - c->y - 2;
				else
					c->h = h - 2;
			}
			else { /* fallback if h < bh */
				c->x = sx + mw;
				c->y = sy + bh;
				c->w = w - 2;
				c->h = sh - 2 - bh;
			}
			resize(c, False, TopLeft);
			i++;
		}
		else
			ban(c);
	}
	if(!sel || !isvisible(sel))
		focus(getnext(clients));
	restack();
}

void
focusnext(Arg *arg)
{
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
focusprev(Arg *arg)
{
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
isvisible(Client *c)
{
	unsigned int i;

	for(i = 0; i < ntags; i++)
		if(c->tags[i] && seltag[i])
			return True;
	return False;
}

void
resizecol(Arg *arg)
{
	unsigned int n;
	Client *c;

	for(n = 0, c = clients; c; c = c->next)
		if(isvisible(c) && !c->isfloat)
			n++;
	if(!sel || sel->isfloat || n < 2 || (arrange != dotile) || maximized)
		return;

	if(sel == getnext(clients)) {
		if(mw + arg->i > sw - 100 || mw + arg->i < 100)
			return;
		mw += arg->i;
	}
	else {
		if(mw - arg->i > sw - 100 || mw - arg->i < 100)
			return;
		mw -= arg->i;
	}
	arrange(NULL);
}

void
restack()
{
	static unsigned int nwins = 0;
	static Window *wins = NULL;
	unsigned int f, fi, m, mi, n;
	Client *c;
	XEvent ev;

	for(f = 0, m = 0, c = clients; c; c = c->next)
		if(isvisible(c)) {
			if(c->isfloat || arrange == dofloat)
				f++;
			else
				m++;
		}
	if(!(n = 2 * (f + m))) {
		drawstatus();
		return;
	}
	if(nwins < n) {
		nwins = n;
		wins = erealloc(wins, nwins * sizeof(Window));
	}

	fi = 0;
	mi = 2 * f;
	if(sel) {
		if(sel->isfloat || arrange == dofloat) {
			wins[fi++] = sel->twin;
			wins[fi++] = sel->win;
		}
		else {
			wins[mi++] = sel->twin;
			wins[mi++] = sel->win;
		}
	}
	for(c = clients; c; c = c->next)
		if(isvisible(c) && c != sel) {
			if(c->isfloat || arrange == dofloat) {
				wins[fi++] = c->twin;
				wins[fi++] = c->win;
			}
			else {
				wins[mi++] = c->twin;
				wins[mi++] = c->win;
			}
		}
	XRestackWindows(dpy, wins, n);
	drawall();
	XSync(dpy, False);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
togglemode(Arg *arg)
{
	arrange = (arrange == dofloat) ? dotile : dofloat;
	if(sel)
		arrange(NULL);
	else
		drawstatus();
}

void
toggleview(Arg *arg)
{
	unsigned int i;

	seltag[arg->i] = !seltag[arg->i];
	for(i = 0; i < ntags && !seltag[i]; i++);
	if(i == ntags)
		seltag[arg->i] = True; /* cannot toggle last view */
	reorder();
	arrange(NULL);
}

void
view(Arg *arg)
{
	unsigned int i;

	for(i = 0; i < ntags; i++)
		seltag[i] = False;
	seltag[arg->i] = True;
	reorder();
	arrange(NULL);
}

void
viewall(Arg *arg)
{
	unsigned int i;

	for(i = 0; i < ntags; i++)
		seltag[i] = True;
	reorder();
	arrange(NULL);
}

void
zoom(Arg *arg)
{
	unsigned int n;
	Client *c;

	for(n = 0, c = clients; c; c = c->next)
		if(isvisible(c) && !c->isfloat)
			n++;
	if(!sel || sel->isfloat || n < 2 || (arrange != dotile) || maximized)
		return;

	if((c = sel) == nexttiled(clients))
		if(!(c = nexttiled(c)))
			return;
	detach(c);
	c->next = clients;
	clients->prev = c;
	clients = c;
	focus(c);
	arrange(NULL);
}
