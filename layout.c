/* See LICENSE file for copyright and license details. */
#include "dwm.h"
#include <stdio.h>
#include <stdlib.h>

unsigned int blw = 0;
Layout *lt = NULL;

/* static */

static double hratio = HRATIO;
static double vratio = VRATIO;
static unsigned int nlayouts = 0;
static unsigned int nmaster = NMASTER;

static void
incratio(const char *arg, double *ratio, double def) {
	double delta;

	if(lt->arrange != tile)
		return;
	if(!arg)
		*ratio = def;
	else {
		if(1 == sscanf(arg, "%lf", &delta)) {
			if(delta + (*ratio) < .1 || delta + (*ratio) > 1.9)
				return;
			*ratio += delta;
		}
	}
	lt->arrange();
}

static double /* simple pow() */
spow(double x, double y)
{
	if(y == 0)
		return 1;
	while(--y)
		x *= x;
	return x;
}

static void
tile(void) {
	double mscale = 0, tscale = 0, sum = 0;
	unsigned int i, n, nx, ny, nw, nh, mw, tw;
	Client *c;

	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next))
		n++;

	mw = (n <= nmaster) ? waw :  waw / (1 + hratio);
	tw = waw - mw;

	if(n > 0) {
		if(n < nmaster) {
			for(i = 0; i < n; i++)
				sum += spow(vratio, i);
			mscale = wah / sum;
		}
		else {
			for(i = 0; i < nmaster; i++)
				sum += spow(vratio, i);
			mscale = wah / sum;
			for(sum = 0, i = 0; i < (n - nmaster); i++)
				sum += spow(vratio, i);
			tscale = wah / sum;
		}
	}
	nx = wax;
	ny = way;
	for(i = 0, c = clients; c; c = c->next)
		if(isvisible(c)) {
			unban(c);
			if(c->isfloating)
				continue;
			c->ismax = False;
			if(i < nmaster) { /* master window */
				nw = mw - 2 * c->border;
				if(i + 1 == n || i + 1 == nmaster)
					nh = (way + wah) - ny - (2 * c->border);
				else
					nh = (mscale * spow(vratio, i)) - (2 * c->border);
			}
			else { /* tile window */
				if(i == nmaster) {
					ny = way;
					nx = wax + mw;
				}
				nw = tw - 2 * c->border;
				if(i + 1 == n)
					nh = (way + wah) - ny - (2 * c->border);
				else
					nh = (tscale * spow(vratio, i - nmaster)) - (2 * c->border);
			}
			if(nh < bh) {
				nh = bh;
				ny = way + wah - nh;
			}
			resize(c, nx, ny, nw, nh, False);
			ny += nh;
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
inchratio(const char *arg) {
	incratio(arg, &hratio, HRATIO);
}

void
incvratio(const char *arg) {
	incratio(arg, &vratio, VRATIO);
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
	lt->arrange();
}
