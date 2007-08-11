/* See LICENSE file for copyright and license details. */
#include "dwm.h"
#include <stdlib.h>

unsigned int blw = 0;
Layout *lt = NULL;

/* static */

static unsigned int nlayouts = 0;

LAYOUTS

/* extern */

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
