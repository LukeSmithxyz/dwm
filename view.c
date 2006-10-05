/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"

#define MINDIM			100

/* static */

static Client *
minclient(void) {
	Client *c, *min;

	if((clients && clients->isfloat) || arrange == dofloat)
		return clients; /* don't touch floating order */
	for(min = c = clients; c; c = c->next)
		if(c->weight < min->weight)
			min = c;
	return min;
}

static Client *
nexttiled(Client *c) {
	for(c = getnext(c); c && c->isfloat; c = getnext(c->next));
	return c;
}

static void
reorder(void) {
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

static void
togglemax(Client *c)
{
	XEvent ev;
	if((c->ismax = !c->ismax)) {
		c->rx = c->x; c->x = sx;
		c->ry = c->y; c->y = bh;
		c->rw = c->w; c->w = sw - 2 * BORDERPX;
		c->rh = c->h; c->h = sh - bh - 2 * BORDERPX;
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

void (*arrange)(Arg *) = DEFMODE;
StackPos stackpos = STACKPOS;

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
dofloat(Arg *arg) {
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

/* This algorithm is based on a (M)aster area and a (S)tacking area.
 * It supports following arrangements:
 * 	SSMMM	MMMMM	MMMSS
 * 	SSMMM	SSSSS	MMMSS
 */
void
dotile(Arg *arg) {
	int i, n, stackw, stackh, tw, th;
	Client *c;

	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next))
		n++;

	if(stackpos == StackBottom) {
		stackw = sw;
		stackh = sh - bh - master;
	}
	else {
		stackw = sw - master;
		stackh = sh - bh;
	}

	tw = stackw;
	if(n > 1)
		th = stackh / (n - 1);
	else
		th = stackh;

	for(i = 0, c = clients; c; c = c->next) {
		if(isvisible(c)) {
			if(c->isfloat) {
				resize(c, True, TopLeft);
				continue;
			}
			c->ismax = False;
			if(n == 1) { /* only 1 window */
				c->x = sx;
				c->y = sy + bh;
				c->w = sw - 2 * BORDERPX;
				c->h = sh - 2 * BORDERPX - bh;
			}
			else if(i == 0) { /* master window */
				c->x = sx;
				if(stackpos == StackLeft)
					c->x += stackw;
				c->y = sy + bh;
				switch(stackpos) {
				case StackLeft:
				case StackRight:
					c->w = master - 2 * BORDERPX;
					c->h = sh - bh - 2 * BORDERPX;
					break;
				case StackBottom:
					c->w = sw - 2 * BORDERPX;
					c->h = master - 2 * BORDERPX;
					break;
				}
			}
			else if(th > bh) { /* tile window */
				c->x = sx;
				if(stackpos == StackRight)
					c->x += master;
				c->w = tw - 2 * BORDERPX;
				c->h = th - 2 * BORDERPX;
				switch(stackpos) {
				case StackLeft:
				case StackRight:
					c->y = sy + (i - 1) * th + bh;
					if(i + 1 == n)
						c->h = sh - c->y - 2 * BORDERPX;
					break;
				case StackBottom:
					c->y = sy + master + (i - 1) * th + bh;
					if(i + 1 == n)
						c->h = sh - c->y - 2 * BORDERPX;
					break;
				}
			}
			else { /* fallback if th < bh */
				c->x = sx;
				if(stackpos == StackRight)
					c->x += master;
				c->y = sy + bh;
				if(stackpos == StackBottom)
					c->y += master;
				c->w = stackw - 2 * BORDERPX;
				c->h = stackh - 2 * BORDERPX;
			}
			resize(c, False, TopLeft);
			i++;
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
resizecol(Arg *arg) {
	int s;
	unsigned int n;
	Client *c;

	for(n = 0, c = clients; c; c = c->next)
		if(isvisible(c) && !c->isfloat)
			n++;
	if(!sel || sel->isfloat || n < 2 || (arrange == dofloat))
		return;

	s = stackpos == StackBottom ? sh - bh : sw;
	if(sel == getnext(clients)) {
		if(master + arg->i > s - MINDIM || master + arg->i < MINDIM)
			return;
		master += arg->i;
	}
	else {
		if(master - arg->i > s - MINDIM || master - arg->i < MINDIM)
			return;
		master -= arg->i;
	}
	arrange(NULL);
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
togglemode(Arg *arg) {
	arrange = (arrange == dofloat) ? dotile : dofloat;
	if(sel)
		arrange(NULL);
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
	reorder();
	arrange(NULL);
}

void
togglestackpos(Arg *arg) {
	if(arrange == dofloat)
		return;
	if(stackpos == StackBottom)
		stackpos = STACKPOS;
	else
		stackpos = StackBottom;
	master = ((stackpos == StackBottom ? sh - bh : sw) * MASTER) / 100;
	arrange(NULL);
}

void
view(Arg *arg) {
	unsigned int i;

	for(i = 0; i < ntags; i++)
		seltag[i] = False;
	seltag[arg->i] = True;
	reorder();
	arrange(NULL);
}

void
viewall(Arg *arg) {
	unsigned int i;

	for(i = 0; i < ntags; i++)
		seltag[i] = True;
	reorder();
	arrange(NULL);
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
	arrange(NULL);
}
