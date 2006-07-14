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
	int i, modw;
	char *mode = arrange == tiling ? "#" : "~";

	dc.x = dc.y = 0;
	dc.w = bw;
	drawtext(NULL, False, False);

	modw = textw(mode) + dc.font.height;
	dc.w = 0;
	for(i = 0; i < TLast; i++) {
		dc.x += dc.w;
		dc.w = textw(tags[i]) + dc.font.height;
		drawtext(tags[i], i == tsel, True);
	}
	if(sel) {
		dc.x += dc.w;
		dc.w = textw(sel->name) + dc.font.height;
		drawtext(sel->name, True, True);
	}
	dc.w = textw(stext) + dc.font.height;
	dc.x = bx + bw - dc.w - modw;
	drawtext(stext, False, False);

	dc.x = bx + bw - modw;
	dc.w = modw;
	drawtext(mode, True, True);

	XCopyArea(dpy, dc.drawable, barwin, dc.gc, 0, 0, bw, bh, 0, 0);
	XFlush(dpy);
}
