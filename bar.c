/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "wm.h"

void
draw_bar()
{
	int i;
	brush.x = brush.y = 0;
	brush.w = bw;
	brush.h = bh;
	draw(dpy, &brush, False, NULL);

	brush.w = 0;
	for(i = 0; i < TLast; i++) {
		brush.x += brush.w;
		brush.w = textw(&brush.font, tags[i]) + bh;
		if(i == tsel) {
			swap((void **)&brush.fg, (void **)&brush.bg);
			draw(dpy, &brush, True, tags[i]);
			swap((void **)&brush.fg, (void **)&brush.bg);
		}
		else
			draw(dpy, &brush, True, tags[i]);
	}
	if(stack) {
		swap((void **)&brush.fg, (void **)&brush.bg);
		brush.x += brush.w;
		brush.w = textw(&brush.font, stack->name) + bh;
		draw(dpy, &brush, True, stack->name);
		swap((void **)&brush.fg, (void **)&brush.bg);
	}
	brush.w = textw(&brush.font, stext) + bh;
	brush.x = bx + bw - brush.w;
	draw(dpy, &brush, False, stext);
	XCopyArea(dpy, brush.drawable, barwin, brush.gc, 0, 0, bw, bh, 0, 0);
	XFlush(dpy);
}
