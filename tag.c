/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"

#include <string.h>
#include <X11/Xutil.h>

/* static */

/* CUSTOMIZE */ 
static Rule rule[] = {
	/* class			instance	tags						isfloat */
	{ "Firefox-bin",	"Gecko",	{ [Twww] = "www" },			False },
};

/* extern */

/* CUSTOMIZE */
char *tags[TLast] = {
	[Tscratch] = "scratch",
	[Tdev] = "dev",
	[Twww] = "www",
	[Twork] = "work",
};
void (*arrange)(Arg *) = dotile;

void
appendtag(Arg *arg)
{
	if(!sel)
		return;

	sel->tags[arg->i] = tags[arg->i];
	arrange(NULL);
}

void
dofloat(Arg *arg)
{
	Client *c;

	arrange = dofloat;
	for(c = clients; c; c = c->next) {
		if(c->tags[tsel])
			resize(c, True);
		else
			ban(c);
	}
	if(sel && !sel->tags[tsel]) {
		if((sel = getnext(clients, tsel))) {
			higher(sel);
			focus(sel);
		}
	}
	drawall();
}

void
dotile(Arg *arg)
{
	Client *c;
	int n, i, w, h;

	w = sw - mw;
	arrange = dotile;
	for(n = 0, c = clients; c; c = c->next)
		if(c->tags[tsel] && !c->isfloat)
			n++;

	if(n > 1)
		h = (sh - bh) / (n - 1);
	else
		h = sh - bh;

	for(i = 0, c = clients; c; c = c->next) {
		if(c->tags[tsel]) {
			if(c->isfloat) {
				higher(c);
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
			ban(c);
	}
	if(!sel || (sel && !sel->tags[tsel])) {
		if((sel = getnext(clients, tsel))) {
			higher(sel);
			focus(sel);
		}
	}
	drawall();
}

Client *
getnext(Client *c, unsigned int t)
{
	for(; c && !c->tags[t]; c = c->next);
	return c;
}

void
heretag(Arg *arg)
{
	int i;
	Client *c;

	if(arg->i == tsel)
		return;

	if(!(c = getnext(clients, arg->i)))
		return;

	for(i = 0; i < TLast; i++)
		c->tags[i] = NULL;
	c->tags[tsel] = tags[tsel];
	pop(c);
	focus(c);
}

void
replacetag(Arg *arg)
{
	int i;
	if(!sel)
		return;

	for(i = 0; i < TLast; i++)
		sel->tags[i] = NULL;
	appendtag(arg);
}

void
settags(Client *c)
{
	XClassHint ch;
	static unsigned int len = rule ? sizeof(rule) / sizeof(rule[0]) : 0;
	unsigned int i, j;
	Bool matched = False;

	if(!len) {
		c->tags[tsel] = tags[tsel];
		return;
	}

	if(XGetClassHint(dpy, c->win, &ch)) {
		if(ch.res_class && ch.res_name) {
			for(i = 0; i < len; i++)
				if(!strncmp(rule[i].class, ch.res_class, sizeof(rule[i].class))
					&& !strncmp(rule[i].instance, ch.res_name, sizeof(rule[i].instance)))
				{
					for(j = 0; j < TLast; j++)
						c->tags[j] = rule[i].tags[j];
					c->isfloat = rule[i].isfloat;
					matched = True;
					break;
				}
		}
		if(ch.res_class)
			XFree(ch.res_class);
		if(ch.res_name)
			XFree(ch.res_name);
	}

	if(!matched)
		c->tags[tsel] = tags[tsel];
}

void
view(Arg *arg)
{
	tsel = arg->i;
	arrange(NULL);
	drawall();
}
