/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "wm.h"
#include <stdio.h>

void
run(char *arg)
{
	spawn(dpy, arg);
}

void
quit(char *arg)
{
	fputs("quit\n", stderr);
	running = False;
}

void
kill(char *arg)
{
	Client *c = stack;

	if(!c)
		return;
	if(c->proto & WM_PROTOCOL_DELWIN)
		send_message(c->win, wm_atom[WMProtocols], wm_atom[WMDelete]);
	else
		XKillClient(dpy, c->win);
}

