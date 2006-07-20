/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"

#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <X11/Xutil.h>

/* static */

typedef struct {
	const char *pattern;
	char *tags[TLast];
	Bool isfloat;
} Rule;

/* CUSTOMIZE */ 
static Rule rule[] = {
	/* class:instance	tags				isfloat */
	{ "Firefox.*",		{ [Twww] = "www" },		False },
	{ "Gimp.*",		{ 0 },				True},
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
		if(c->tags[tsel]) {
			resize(c, True, TopLeft);
		}
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
	int n, i, w, h;
	Client *c;

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
				resize(c, True, TopLeft);
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
			else if(h > bh) {
				c->x = sx + mw;
				c->y = sy + (i - 1) * h + bh;
				c->w = w - 2 * c->border;
				c->h = h - 2 * c->border;
			}
			else { /* fallback if h < bh */
				c->x = sx + mw;
				c->y = sy + bh;
				c->w = w - 2 * c->border;
				c->h = sh - 2 * c->border - bh;
			}
			resize(c, False, TopLeft);
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
	char classinst[256];
	static unsigned int len = rule ? sizeof(rule) / sizeof(rule[0]) : 0;
	unsigned int i, j;
	regex_t regex;
	regmatch_t tmp;
	Bool matched = False;
	XClassHint ch;

	if(!len) {
		c->tags[tsel] = tags[tsel];
		return;
	}

	if(XGetClassHint(dpy, c->win, &ch)) {
		snprintf(classinst, sizeof(classinst), "%s:%s",
				ch.res_class ? ch.res_class : "",
				ch.res_name ? ch.res_name : "");
		for(i = 0; !matched && i < len; i++) {
			if(!regcomp(&regex, rule[i].pattern, 0)) {
				if(!regexec(&regex, classinst, 1, &tmp, 0)) {
					for(j = 0; j < TLast; j++) {
						if(rule[i].tags[j])
							matched = True;
						c->tags[j] = rule[i].tags[j];
					}
					c->isfloat = rule[i].isfloat;
				}
				regfree(&regex);
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
