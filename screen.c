/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "dwm.h"

void (*arrange)(Arg *) = tiling;

void
view(Arg *arg)
{
	Client *c;

	tsel = arg->i;
	arrange(NULL);

	for(c = clients; c; c = next(c->next))
		draw_client(c);
	draw_bar();
}

void
floating(Arg *arg)
{
	Client *c;

	arrange = floating;
	for(c = clients; c; c = c->next) {
		if(c->tags[tsel])
			resize(c, True);
		else
			ban_client(c);
	}
	if(sel && !sel->tags[tsel]) {
		if((sel = next(clients))) {
			craise(sel);
			focus(sel);
		}
	}
	draw_bar();
}

void
tiling(Arg *arg)
{
	Client *c;
	int n, i, w, h;

	w = sw - mw;
	arrange = tiling;
	for(n = 0, c = clients; c; c = c->next)
		if(c->tags[tsel] && !c->floating)
			n++;

	if(n > 1)
		h = (sh - bh) / (n - 1);
	else
		h = sh - bh;

	for(i = 0, c = clients; c; c = c->next) {
		if(c->tags[tsel]) {
			if(c->floating) {
				craise(c);
				resize(c, True);
				continue;
			}
			if(n == 1) {
				c->x = sx;
				c->y = sy + bh;
				c->w = sw - 2 * c->border;
				c->h = sh - 2 * c->border - bh;
			}
			else if(i == 0) {
				c->x = sx;
				c->y = sy + bh;
				c->w = mw - 2 * c->border;
				c->h = sh - 2 * c->border - bh;
			}
			else {
				c->x = sx + mw;
				c->y = sy + (i - 1) * h + bh;
				c->w = w - 2 * c->border;
				c->h = h - 2 * c->border;
			}
			resize(c, False);
			i++;
		}
		else
			ban_client(c);
	}
	if(!sel || (sel && !sel->tags[tsel])) {
		if((sel = next(clients))) {
			craise(sel);
			focus(sel);
		}
	}
	draw_bar();
}

