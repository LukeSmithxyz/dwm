/* (C)opyright MMVI-MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"

unsigned int master = MASTER;
unsigned int nmaster = NMASTER;

/* static */

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

void
dotile(void) {
	unsigned int i, n, nx, ny, nw, nh, mw, mh, tw, th;
	Client *c;

	for(n = 0, c = nextmanaged(clients); c; c = nextmanaged(c->next))
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

void
resizemaster(Arg *arg) {
	if(arrange != dotile)
		return;
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
zoom(Arg *arg) {
	unsigned int n;
	Client *c;

	if(!sel)
		return;
	if(sel->isfloat || (arrange == dofloat)) {
		togglemax(sel);
		return;
	}
	for(n = 0, c = nextmanaged(clients); c; c = nextmanaged(c->next))
		n++;

	if((c = sel) == nextmanaged(clients))
		if(!(c = nextmanaged(c->next)))
			return;
	detach(c);
	attach(c);
	focus(c);
	arrange();
}
