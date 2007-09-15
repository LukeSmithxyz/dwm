/* See LICENSE file for copyright and license details. */
#include "dwm.h"
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xutil.h>

/* static */

typedef struct {
	const char *symbol;
	void (*arrange)(void);
} Layout;

typedef struct {
	const char *prop;
	const char *tags;
	Bool isfloating;
} Rule;

typedef struct {
	regex_t *propregex;
	regex_t *tagregex;
} Regs;

TAGS
RULES

static unsigned int nrules = 0;
static unsigned int nlayouts = 0;
static unsigned int ltidx = 0; /* default */
static Regs *regs = NULL;

static unsigned int
idxoftag(const char *tag) {
	unsigned int i;

	for(i = 0; i < ntags; i++)
		if(tags[i] == tag)
			return i;
	return 0;
}

static void
floating(void) { /* default floating layout */
	Client *c;

	for(c = clients; c; c = c->next)
		if(isvisible(c))
			resize(c, c->x, c->y, c->w, c->h, True);
}

LAYOUTS

/* extern */

unsigned int blw = 0;

void
applyrules(Client *c) {
	static char buf[512];
	unsigned int i, j;
	regmatch_t tmp;
	Bool matched = False;
	XClassHint ch = { 0 };

	/* rule matching */
	XGetClassHint(dpy, c->win, &ch);
	snprintf(buf, sizeof buf, "%s:%s:%s",
			ch.res_class ? ch.res_class : "",
			ch.res_name ? ch.res_name : "", c->name);
	for(i = 0; i < nrules; i++)
		if(regs[i].propregex && !regexec(regs[i].propregex, buf, 1, &tmp, 0)) {
			c->isfloating = rules[i].isfloating;
			for(j = 0; regs[i].tagregex && j < ntags; j++) {
				if(!regexec(regs[i].tagregex, tags[j], 1, &tmp, 0)) {
					matched = True;
					c->tags[j] = True;
				}
			}
		}
	if(ch.res_class)
		XFree(ch.res_class);
	if(ch.res_name)
		XFree(ch.res_name);
	if(!matched)
		for(i = 0; i < ntags; i++)
			c->tags[i] = seltags[i];
}

void
arrange(void) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(isvisible(c))
			unban(c);
		else
			ban(c);
	layouts[ltidx].arrange();
	focus(NULL);
	restack();
}

void
compileregs(void) {
	unsigned int i;
	regex_t *reg;

	if(regs)
		return;
	nrules = sizeof rules / sizeof rules[0];
	regs = emallocz(nrules * sizeof(Regs));
	for(i = 0; i < nrules; i++) {
		if(rules[i].prop) {
			reg = emallocz(sizeof(regex_t));
			if(regcomp(reg, rules[i].prop, REG_EXTENDED))
				free(reg);
			else
				regs[i].propregex = reg;
		}
		if(rules[i].tags) {
			reg = emallocz(sizeof(regex_t));
			if(regcomp(reg, rules[i].tags, REG_EXTENDED))
				free(reg);
			else
				regs[i].tagregex = reg;
		}
	}
}

void
focusnext(const char *arg) {
	Client *c;

	if(!sel)
		return;
	for(c = sel->next; c && !isvisible(c); c = c->next);
	if(!c)
		for(c = clients; c && !isvisible(c); c = c->next);
	if(c) {
		focus(c);
		restack();
	}
}

void
focusprev(const char *arg) {
	Client *c;

	if(!sel)
		return;
	for(c = sel->prev; c && !isvisible(c); c = c->prev);
	if(!c) {
		for(c = clients; c && c->next; c = c->next);
		for(; c && !isvisible(c); c = c->prev);
	}
	if(c) {
		focus(c);
		restack();
	}
}

const char *
getsymbol(void)
{
	return layouts[ltidx].symbol;
}

void
initlayouts(void) {
	unsigned int i, w;

	nlayouts = sizeof layouts / sizeof layouts[0];
	for(blw = i = 0; i < nlayouts; i++) {
		w = textw(layouts[i].symbol);
		if(w > blw)
			blw = w;
	}
}

Bool
isfloating(void) {
	return layouts[ltidx].arrange == floating;
}

Bool
isarrange(void (*func)())
{
	return func == layouts[ltidx].arrange;
}

Bool
isvisible(Client *c) {
	unsigned int i;

	for(i = 0; i < ntags; i++)
		if(c->tags[i] && seltags[i])
			return True;
	return False;
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

	drawbar();
	if(!sel)
		return;
	if(sel->isfloating || isfloating())
		XRaiseWindow(dpy, sel->win);
	if(!isfloating()) {
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
	unsigned int i;

	if(!arg) {
		if(++ltidx == nlayouts)
			ltidx = 0;;
	}
	else {
		for(i = 0; i < nlayouts; i++)
			if(!strcmp(arg, layouts[i].symbol))
				break;
		if(i == nlayouts)
			return;
		ltidx = i;
	}
	if(sel)
		arrange();
	else
		drawbar();
}

void
tag(const char *arg) {
	unsigned int i;

	if(!sel)
		return;
	for(i = 0; i < ntags; i++)
		sel->tags[i] = arg == NULL;
	i = idxoftag(arg);
	if(i >= 0 && i < ntags)
		sel->tags[i] = True;
	arrange();
}

void
togglefloating(const char *arg) {
	if(!sel)
		return;
	sel->isfloating = !sel->isfloating;
	if(sel->isfloating)
		resize(sel, sel->x, sel->y, sel->w, sel->h, True);
	arrange();
}

void
togglemax(const char *arg) {
	XEvent ev;

	if(!sel || (!isfloating() && !sel->isfloating) || sel->isfixed)
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
	drawbar();
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
toggletag(const char *arg) {
	unsigned int i, j;

	if(!sel)
		return;
	i = idxoftag(arg);
	sel->tags[i] = !sel->tags[i];
	for(j = 0; j < ntags && !sel->tags[j]; j++);
	if(j == ntags)
		sel->tags[i] = True;
	arrange();
}

void
toggleview(const char *arg) {
	unsigned int i, j;

	i = idxoftag(arg);
	seltags[i] = !seltags[i];
	for(j = 0; j < ntags && !seltags[j]; j++);
	if(j == ntags)
		seltags[i] = True; /* cannot toggle last view */
	arrange();
}

void
view(const char *arg) {
	unsigned int i;

	for(i = 0; i < ntags; i++)
		seltags[i] = arg == NULL;
	i = idxoftag(arg);
	if(i >= 0 && i < ntags)
		seltags[i] = True;
	arrange();
}
