/* See LICENSE file for copyright and license details. */
#include "dwm.h"
#include <stdio.h>
#include <stdlib.h>

unsigned int blw = 0;
Layout *lt = NULL;

/* static */

static unsigned int nlayouts = 0;

LAYOUTS

/* extern */

void
floating(const char *arg) {
	Client *c;

	if(lt->arrange != floating)
		return;

	for(c = clients; c; c = c->next)
		if(isvisible(c)) {
			unban(c);
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
	XWindowChanges wc;

	drawstatus();
	if(!sel)
		return;
	if(sel->isfloating || lt->arrange == floating)
		XRaiseWindow(dpy, sel->win);
	if(lt->arrange != floating) {
		wc.stack_mode = Below;
		wc.sibling = barwin;
		if(!sel->isfloating) {
			XConfigureWindow(dpy, sel->win, CWSibling | CWStackMode, &wc);
			wc.sibling = sel->win;
		}
		for(c = nexttiled(clients); c; c = nexttiled(c->next)) {
			if(c == sel)
				continue;
			XConfigureWindow(dpy, c->win, CWSibling | CWStackMode, &wc);
			wc.sibling = c->win;
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
		lt->arrange(NULL);
	else
		drawstatus();
}

void
tile(const char *arg) {
	static double master = MASTER;
	double delta;
	unsigned int i, n, nx, ny, nw, nh, mw, th;
	Client *c;

	if(lt->arrange != tile)
		return;

	/* arg handling, manipulate master */
	if(arg && (1 == sscanf(arg, "%lf", &delta))) {
		if(delta + master > 0.1 && delta + master < 0.9)
			master += delta;
	}

	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next))
		n++;

	/* window geoms */
	mw = (n == 1) ? waw : master * waw;
	th = (n > 1) ? wah / (n - 1) : 0;
	if(n > 1 && th < bh)
		th = wah;

	nx = wax;
	ny = way;
	for(i = 0, c = clients; c; c = c->next)
		if(isvisible(c)) {
			unban(c);
			if(c->isfloating)
				continue;
			c->ismax = False;
			if(i == 0) { /* master */
				nw = mw - 2 * c->border;
				nh = wah - 2 * c->border;
			}
			else {  /* tile window */
				if(i == 1) {
					ny = way;
					nx += mw;
				}
				nw = waw - mw - 2 * c->border;
				if(i + 1 == n) /* remainder */
					nh = (way + wah) - ny - 2 * c->border;
				else
					nh = th - 2 * c->border;
			}
			resize(c, nx, ny, nw, nh, False);
			if(n > 1 && th != wah)
				ny += nh;
			i++;
		}
		else
			ban(c);
	focus(NULL);
	restack();
}

void
togglebar(const char *arg) {
	if(bpos == BarOff)
		bpos = (BARPOS == BarOff) ? BarTop : BARPOS;
	else
		bpos = BarOff;
	updatebarpos();
	lt->arrange(NULL);
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
		resize(sel, wax, way, waw - 2 * sel->border, wah - 2 * sel->border, True);
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
	lt->arrange(NULL);
}
