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
