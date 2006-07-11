/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "wm.h"
#include <stdio.h>
#include <string.h>

void
run(void *aux)
{
	spawn(dpy, aux);
}

void
quit(void *aux)
{
	running = False;
}

void
sel(void *aux)
{
	const char *arg = aux;
	Client *c;

	if(!arg || !stack)
		return;
	if(!strncmp(arg, "next", 5))
		focus(stack->snext ? stack->snext : stack);
	else if(!strncmp(arg, "prev", 5)) {
		for(c = stack; c && c->snext; c = c->snext);
		focus(c ? c : stack);
	}
}

void
kill(void *aux)
{
	Client *c = stack;

	if(!c)
		return;
	if(c->proto & WM_PROTOCOL_DELWIN)
		send_message(c->win, wm_atom[WMProtocols], wm_atom[WMDelete]);
	else
		XKillClient(dpy, c->win);
}

