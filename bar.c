/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "dwm.h"

void
barclick(XButtonPressedEvent *e)
{
	int x = 0;
	Arg a;
	for(a.i = 0; a.i < TLast; a.i++) {
		x += textw(tags[a.i]) + dc.font.height;
		if(e->x < x) {
			view(&a);
			return;
		}
	}
}

void
draw_bar()
{
	int i;
	dc.x = dc.y = 0;
	dc.w = bw;
	drawtext(NULL, False);

	dc.w = 0;
	for(i = 0; i < TLast; i++) {
		dc.x += dc.w;
		dc.w = textw(tags[i]) + dc.font.height;
		if(i == tsel) {
			swap((void **)&dc.fg, (void **)&dc.bg);
			drawtext(tags[i], True);
			swap((void **)&dc.fg, (void **)&dc.bg);
		}
		else
			drawtext(tags[i], True);
	}
	if(sel) {
		swap((void **)&dc.fg, (void **)&dc.bg);
		dc.x += dc.w;
		dc.w = textw(sel->name) + dc.font.height;
		drawtext(sel->name, True);
		swap((void **)&dc.fg, (void **)&dc.bg);
	}
	dc.w = textw(stext) + dc.font.height;
	dc.x = bx + bw - dc.w;
	drawtext(stext, False);
	XCopyArea(dpy, dc.drawable, barwin, dc.gc, 0, 0, bw, bh, 0, 0);
	XFlush(dpy);
}
