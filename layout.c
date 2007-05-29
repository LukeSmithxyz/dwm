/* © 2006-2007 Anselm R. Garbe <garbeam at gmail dot com>
 * © 2006-2007 Sander van Dijk <a dot h dot vandijk at gmail dot com>
 * See LICENSE file for license details. */
#include "dwm.h"
#include <stdlib.h>

unsigned int blw = 0;
Layout *lt = NULL;

/* static */

static unsigned int nlayouts = 0;
static unsigned int masterw = MASTERWIDTH;
static unsigned int nmaster = NMASTER;

static void
ban(Client *c) {
	if (c->isbanned)
		return;
	XMoveWindow(dpy, c->win, c->x + 2 * sw, c->y);
	c->isbanned = True;
}

static void
unban(Client *c) {
	if (!c->isbanned)
		return;
	XMoveWindow(dpy, c->win, c->x, c->y);
	c->isbanned = False;
}

static void
tile(void) {
	unsigned int i, n, nx, ny, nw, nh, mw, mh, tw, th;
	Client *c;

	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next))
		n++;
	/* window geoms */
	mh = (n > nmaster) ? wah / nmaster : wah / (n > 0 ? n : 1);
	mw = (n > nmaster) ? (waw * masterw) / 1000 : waw;
	th = (n > nmaster) ? wah / (n - nmaster) : 0;
	tw = waw - mw;

	for(i = 0, c = clients; c; c = c->next)
		if(isvisible(c)) {
			unban(c);
			if(c->isfloating)
				continue;
			c->ismax = False;
			nx = wax;
			ny = way;
			if(i < nmaster) {
				ny += i * mh;
				nw = mw - 2 * c->border;
				nh = mh;
				if(i + 1 == (n < nmaster ? n : nmaster)) /* remainder */
					nh = wah - mh * i;
				nh -= 2 * c->border;
			}
			else {  /* tile window */
				nx += mw;
				nw = tw - 2 * c->border;
				if(th > 2 * c->border) {
					ny += (i - nmaster) * th;
					nh = th;
					if(i + 1 == n) /* remainder */
						nh = wah - th * (i - nmaster);
					nh -= 2 * c->border;
				}
				else /* fallback if th <= 2 * c->border */
					nh = wah - 2 * c->border;
			}
			resize(c, nx, ny, nw, nh, False);
			i++;
		}
		else
			ban(c);
	focus(NULL);
	restack();
}

LAYOUTS

/* extern */

void
floating(void) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(isvisible(c)) {
			if(c->isbanned)
				XMoveWindow(dpy, c->win, c->x, c->y);
			c->isbanned = False;
			resize(c, c->x, c->y, c->w, c->h, True);
		}
		else
			ban(c);
	focus(NULL);
	restack();
}

void
focusclient(const char *arg) {
	Client *c;
   
	if(!sel || !arg)
		return;
	if(atoi(arg) < 0) {
		for(c = sel->prev; c && !isvisible(c); c = c->prev);
		if(!c) {
			for(c = clients; c && c->next; c = c->next);
			for(; c && !isvisible(c); c = c->prev);
		}
	}
	else {
		for(c = sel->next; c && !isvisible(c); c = c->next);
		if(!c)
			for(c = clients; c && !isvisible(c); c = c->next);
	}
	if(c) {
		focus(c);
		restack();
	}
}

void
incmasterw(const char *arg) {
	int i;
	if(lt->arrange != tile)
		return;
	if(!arg)
		masterw = MASTERWIDTH;
	else {
		i = atoi(arg);
		if(waw * (masterw + i) / 1000 >= waw - 2 * BORDERPX 
		|| waw * (masterw + i) / 1000 <= 2 * BORDERPX)
			return;
		masterw += i;
	}
	lt->arrange();
}

void
incnmaster(const char *arg) {
	int i;

	if(!arg)
		nmaster = NMASTER;
	else {
		i = atoi(arg);
		if((lt->arrange != tile) || (nmaster + i < 1)
		|| (wah / (nmaster + i) <= 2 * BORDERPX))
			return;
		nmaster += i;
	}
	if(sel)
		lt->arrange();
	else
		drawstatus();
}

void
initlayouts(void) {
	unsigned int i, w;

	lt = &layout[0];
	nlayouts = sizeof layout / sizeof layout[0];
	for(blw = i = 0; i < nlayouts; i++) {
		w = textw(layout[i].symbol);
		if(w > blw)
			blw = w;
	}
}

Client *
nexttiled(Client *c) {
	for(; c && (c->isfloating || !isvisible(c)); c = c->next);
	return c;
}

void
restack(void) {
	Client *c;
	XEvent ev;

	drawstatus();
	if(!sel)
		return;
	if(sel->isfloating || lt->arrange == floating)
		XRaiseWindow(dpy, sel->win);
	if(lt->arrange != floating) {
		if(!sel->isfloating)
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
setlayout(const char *arg) {
	int i;

	if(!arg) {
		lt++;
		if(lt == layout + nlayouts)
			lt = layout;
	}
	else {
		i = atoi(arg);
		if(i < 0 || i >= nlayouts)
			return;
		lt = &layout[i];
	}
	if(sel)
		lt->arrange();
	else
		drawstatus();
}

void
togglebar(const char *arg) {
	if(bpos == BarOff)
		bpos = (BARPOS == BarOff) ? BarTop : BARPOS;
	else
		bpos = BarOff;
	updatebarpos();
	lt->arrange();
}

void
togglemax(const char *arg) {
	XEvent ev;

	if(!sel || (lt->arrange != floating && !sel->isfloating) || sel->isfixed)
		return;
	if((sel->ismax = !sel->ismax)) {
		sel->rx = sel->x;
		sel->ry = sel->y;
		sel->rw = sel->w;
		sel->rh = sel->h;
		resize(sel, wax, way, waw - 2 * BORDERPX, wah - 2 * BORDERPX, True);
	}
	else
		resize(sel, sel->rx, sel->ry, sel->rw, sel->rh, True);
	drawstatus();
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
zoom(const char *arg) {
	Client *c;

	if(!sel || lt->arrange == floating || sel->isfloating)
		return;
	if((c = sel) == nexttiled(clients))
		if(!(c = nexttiled(c->next)))
			return;
	detach(c);
	attach(c);
	focus(c);
	lt->arrange();
}
