/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "wm.h"

void
draw_bar()
{
	brush.x = brush.y = 0;
	brush.w = bw;
	brush.h = bh;
	draw(dpy, &brush, False, NULL);

	if(stack) {
		brush.w = textw(&brush.font, stack->name) + bh;
		swap((void **)&brush.fg, (void **)&brush.bg);
		draw(dpy, &brush, True, stack->name);
		swap((void **)&brush.fg, (void **)&brush.bg);
		brush.x += brush.w;
	}

	brush.w = textw(&brush.font, statustext) + bh;
	brush.x = bx + bw - brush.w;
	draw(dpy, &brush, False, statustext);
	XCopyArea(dpy, brush.drawable, barwin, brush.gc, 0, 0, bw, bh, 0, 0);
	XFlush(dpy);
}
