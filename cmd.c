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

