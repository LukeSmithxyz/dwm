/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "wm.h"

void
draw_bar()
{
	brush.rect = barrect;
	brush.rect.x = brush.rect.y = 0;
	draw(dpy, &brush, False, NULL);

	if(stack) {
		brush.rect.width = textwidth(&brush.font, stack->name) + labelheight(&brush.font);
		swap((void **)&brush.fg, (void **)&brush.bg);
		draw(dpy, &brush, True, stack->name);
		swap((void **)&brush.fg, (void **)&brush.bg);
		brush.rect.x += brush.rect.width;
	}

	brush.rect.width = textwidth(&brush.font, statustext) + labelheight(&brush.font);
	brush.rect.x = barrect.x + barrect.width - brush.rect.width;
	draw(dpy, &brush, False, statustext);

	XCopyArea(dpy, brush.drawable, barwin, brush.gc, 0, 0, barrect.width,
			barrect.height, 0, 0);
	XFlush(dpy);
}
