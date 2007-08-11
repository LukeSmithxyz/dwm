/* See LICENSE file for copyright and license details. */
#include "dwm.h"
#include <stdio.h>

/* static */

static double master = MASTER;

/* extern */

void
incmaster(const char *arg) {
	double delta;

	if(lt->arrange != tile)
		return;

	/* arg handling, manipulate master */
	if(arg && (1 == sscanf(arg, "%lf", &delta))) {
		if(delta + master > 0.1 && delta + master < 0.9)
			master += delta;
	}

	lt->arrange();
}

void
tile(void) {
	unsigned int i, n, nx, ny, nw, nh, mw, th;
	Client *c;

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
				ny += nh + 2 * c->border;
			i++;
		}
		else
			ban(c);
	focus(NULL);
	restack();
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
