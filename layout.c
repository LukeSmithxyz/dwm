/* See LICENSE file for copyright and license details. */
#include "dwm.h"
#include <stdlib.h>

/* static */

typedef struct {
	const char *symbol;
	void (*arrange)(void);
} Layout;

unsigned int blw = 0;
static unsigned int sellayout = 0; /* default */

static void
floating(void) { /* default floating layout */
	Client *c;

	for(c = clients; c; c = c->next)
		if(isvisible(c))
			resize(c, c->x, c->y, c->w, c->h, True);
}

static unsigned int nlayouts = 0;

LAYOUTS

/* extern */

void
arrange(void) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(isvisible(c))
			unban(c);
		else
			ban(c);
	layouts[sellayout].arrange();
	focus(NULL);
	restack();
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
	return layouts[sellayout].symbol;
}

Bool
isfloating(void) {
	return layouts[sellayout].arrange == floating;
}

Bool
isarrange(void (*func)())
{
	return func == layouts[sellayout].arrange;
}

void
initlayouts(void) {
	unsigned int i, w;

	/* TODO deserialize sellayout if present */
	nlayouts = sizeof layouts / sizeof layouts[0];
	for(blw = i = 0; i < nlayouts; i++) {
		w = textw(layouts[i].symbol);
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
	int i;

	if(!arg) {
		if(++sellayout == nlayouts)
			sellayout = 0;;
	}
	else {
		i = atoi(arg);
		if(i < 0 || i >= nlayouts)
			return;
		sellayout = i;
	}
	if(sel)
		arrange();
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
	drawstatus();
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}
