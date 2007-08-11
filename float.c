/* See LICENSE file for copyright and license details. */
#include "dwm.h"

/* extern */

void
floating(void) {
	Client *c;

	if(lt->arrange != floating)
		return;

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
