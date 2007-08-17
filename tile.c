/* See LICENSE file for copyright and license details. */
#include "dwm.h"
#include <stdio.h>

/* static */

static double mwfact = MWFACT;

/* extern */

void
setmwfact(const char *arg) {
	double delta, newfact;

	if(!isarrange(tile))
		return;
	/* arg handling, manipulate mwfact */
	if(arg == NULL)
		mwfact = MWFACT;
	else if(1 == sscanf(arg, "%lf", &delta)) {
		if(arg[0] != '+' && arg[0] != '-')
			newfact = delta;
		else
			newfact = mwfact + delta;
		if(newfact < 0.1)
			newfact = 0.1;
		else if(newfact > 0.9)
			newfact = 0.9;
		mwfact = newfact;
	}
	arrange();
}

void
tile(void) {
	unsigned int i, n, nx, ny, nw, nh, mw, th;
	Client *c;

	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next))
		n++;

	/* window geoms */
	mw = (n == 1) ? waw : mwfact * waw;
	th = (n > 1) ? wah / (n - 1) : 0;
	if(n > 1 && th < bh)
		th = wah;

	nx = wax;
	ny = way;
	for(i = 0, c = nexttiled(clients); c; c = nexttiled(c->next)) {
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
			ny += nh + 2 * c->border;
		i++;
	}
}

void
zoom(const char *arg) {
	Client *c;

	if(!sel || !isarrange(tile) || sel->isfloating)
		return;
	if((c = sel) == nexttiled(clients))
		if(!(c = nexttiled(c->next)))
			return;
	detach(c);
	attach(c);
	focus(c);
	arrange();
}
