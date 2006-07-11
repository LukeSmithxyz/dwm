/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "wm.h"

static const char *status[] = {
	"sh", "-c", "echo -n `date` `uptime | sed 's/.*://; s/,//g'`"
		" `acpi | awk '{print $4}' | sed 's/,//'`", 0 \
};

void
draw_bar()
{
	static char buf[1024];

	buf[0] = 0;
	pipe_spawn(buf, sizeof(buf), dpy, (char **)status);

	brush.rect = barrect;
	brush.rect.x = brush.rect.y = 0;
	draw(dpy, &brush, False, buf);

	XCopyArea(dpy, brush.drawable, barwin, brush.gc, 0, 0, barrect.width,
			barrect.height, 0, 0);
	XFlush(dpy);
}
