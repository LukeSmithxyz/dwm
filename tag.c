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
	const unsigned int *tags;
	Bool isfloat;
} Rule;

TAGS
RULES

void (*arrange)(Arg *) = DEFMODE;

/* extern */

void
appendtag(Arg *arg)
{
	if(!sel)
		return;

	sel->tags[arg->i] = True;
	arrange(NULL);
}

void
dofloat(Arg *arg)
{
	Client *c;

	for(c = clients; c; c = c->next) {
		c->ismax = False;
		if(c->tags[tsel]) {
			resize(c, True, TopLeft);
		}
		else
			ban(c);
	}
	if(sel && !sel->tags[tsel]) {
		if((sel = getnext(clients))) {
			higher(sel);
			focus(sel);
		}
		else
			XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
	}
	drawall();
}

void
dotile(Arg *arg)
{
	int n, i, w, h;
	Client *c;

	w = sw - mw;
	for(n = 0, c = clients; c; c = c->next)
		if(c->tags[tsel] && !c->isfloat)
			n++;

	if(n > 1)
		h = (sh - bh) / (n - 1);
	else
		h = sh - bh;

	for(i = 0, c = clients; c; c = c->next) {
		c->ismax = False;
		if(c->tags[tsel]) {
			if(c->isfloat) {
				higher(c);
				resize(c, True, TopLeft);
				continue;
			}
			if(n == 1) {
				c->x = sx;
				c->y = sy + bh;
				c->w = sw - 2;
				c->h = sh - 2 - bh;
			}
			else if(i == 0) {
				c->x = sx;
				c->y = sy + bh;
				c->w = mw - 2;
				c->h = sh - 2 - bh;
			}
			else if(h > bh) {
				c->x = sx + mw;
				c->y = sy + (i - 1) * h + bh;
				c->w = w - 2;
				c->h = h - 2;
			}
			else { /* fallback if h < bh */
				c->x = sx + mw;
				c->y = sy + bh;
				c->w = w - 2;
				c->h = sh - 2 - bh;
			}
			resize(c, False, TopLeft);
			i++;
		}
		else
			ban(c);
	}
	if(!sel || (sel && !sel->tags[tsel])) {
		if((sel = getnext(clients))) {
			higher(sel);
			focus(sel);
		}
		else
			XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
	}
	drawall();
}

Client *
getnext(Client *c)
{
	for(; c && !c->tags[tsel]; c = c->next);
	return c;
}

Client *
getprev(Client *c)
{
	for(; c && !c->tags[tsel]; c = c->prev);
	return c;
}

void
replacetag(Arg *arg)
{
	int i;

	if(!sel)
		return;

	for(i = 0; i < ntags; i++)
		sel->tags[i] = False;
	appendtag(arg);
}

void
settags(Client *c)
{
	char classinst[256];
	static unsigned int len = sizeof(rule) / sizeof(rule[0]);
	unsigned int i, j, n;
	regex_t regex;
	regmatch_t tmp;
	Bool matched = False;
	XClassHint ch;

	if(XGetClassHint(dpy, c->win, &ch)) {
		snprintf(classinst, sizeof(classinst), "%s:%s",
				ch.res_class ? ch.res_class : "",
				ch.res_name ? ch.res_name : "");
		for(i = 0; !matched && i < len; i++) {
			if(!regcomp(&regex, rule[i].pattern, 0)) {
				if(!regexec(&regex, classinst, 1, &tmp, 0)) {
					n = rule[i].tags ?
						sizeof(rule[i].tags) / sizeof(rule[i].tags[0]) : 0;
					matched = n != 0;
					for(j = 0; j < n; j++)
						c->tags[rule[i].tags[j]] = True;
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
		c->tags[tsel] = True;
}

void
togglemode(Arg *arg)
{
	arrange = arrange == dofloat ? dotile : dofloat;
	arrange(NULL);
}

void
view(Arg *arg)
{
	tsel = arg->i;
	arrange(NULL);
	drawall();
}

void
viewnext(Arg *arg)
{
	arg->i = (tsel < ntags-1) ? tsel+1 : 0;
	view(arg);
}

void
viewprev(Arg *arg)
{
	arg->i = (tsel > 0) ? tsel-1 : ntags-1;
	view(arg);
}
