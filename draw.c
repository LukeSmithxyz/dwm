/* © 2004-2007 Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details. */
#include "dwm.h"
#include <string.h>

/* static */

static void
drawsquare(Bool filled, Bool empty, unsigned long col[ColLast]) {
	int x;
	XGCValues gcv;
	XRectangle r = { dc.x, dc.y, dc.w, dc.h };

	gcv.foreground = col[ColFG];
	XChangeGC(dpy, dc.gc, GCForeground, &gcv);
	x = (dc.font.ascent + dc.font.descent + 2) / 4;
	r.x = dc.x + 1;
	r.y = dc.y + 1;
	if(filled) {
		r.width = r.height = x + 1;
		XFillRectangles(dpy, dc.drawable, dc.gc, &r, 1);
	}
	else if(empty) {
		r.width = r.height = x;
		XDrawRectangles(dpy, dc.drawable, dc.gc, &r, 1);
	}
}

static Bool
isoccupied(unsigned int t) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(c->tags[t])
			return True;
	return False;
}

static unsigned int
textnw(const char *text, unsigned int len) {
	XRectangle r;

	if(dc.font.set) {
		XmbTextExtents(dc.font.set, text, len, NULL, &r);
		return r.width;
	}
	return XTextWidth(dc.font.xfont, text, len);
}

/* extern */

void
drawstatus(void) {
	int i, x;

	dc.x = dc.y = 0;
	for(i = 0; i < ntags; i++) {
		dc.w = textw(tags[i]);
		if(seltag[i]) {
			drawtext(tags[i], dc.sel);
			drawsquare(sel && sel->tags[i], isoccupied(i), dc.sel);
		}
		else {
			drawtext(tags[i], dc.norm);
			drawsquare(sel && sel->tags[i], isoccupied(i), dc.norm);
		}
		dc.x += dc.w;
	}
	dc.w = blw;
	drawtext(lt->symbol, dc.norm);
	x = dc.x + dc.w;
	dc.w = textw(stext);
	dc.x = sw - dc.w;
	if(dc.x < x) {
		dc.x = x;
		dc.w = sw - x;
	}
	drawtext(stext, dc.norm);
	if((dc.w = dc.x - x) > bh) {
		dc.x = x;
		if(sel) {
			drawtext(sel->name, dc.sel);
			drawsquare(sel->ismax, sel->isfloating, dc.sel);
		}
		else
			drawtext(NULL, dc.norm);
	}
	XCopyArea(dpy, dc.drawable, barwin, dc.gc, 0, 0, sw, bh, 0, 0);
	XSync(dpy, False);
}

void
drawtext(const char *text, unsigned long col[ColLast]) {
	int x, y, w, h;
	static char buf[256];
	unsigned int len, olen;
	XRectangle r = { dc.x, dc.y, dc.w, dc.h };

	XSetForeground(dpy, dc.gc, col[ColBG]);
	XFillRectangles(dpy, dc.drawable, dc.gc, &r, 1);
	if(!text)
		return;
	w = 0;
	olen = len = strlen(text);
	if(len >= sizeof buf)
		len = sizeof buf - 1;
	memcpy(buf, text, len);
	buf[len] = 0;
	h = dc.font.ascent + dc.font.descent;
	y = dc.y + (dc.h / 2) - (h / 2) + dc.font.ascent;
	x = dc.x + (h / 2);
	/* shorten text if necessary */
	while(len && (w = textnw(buf, len)) > dc.w - h)
		buf[--len] = 0;
	if(len < olen) {
		if(len > 1)
			buf[len - 1] = '.';
		if(len > 2)
			buf[len - 2] = '.';
		if(len > 3)
			buf[len - 3] = '.';
	}
	if(w > dc.w)
		return; /* too long */
	XSetForeground(dpy, dc.gc, col[ColFG]);
	if(dc.font.set)
		XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, x, y, buf, len);
	else
		XDrawString(dpy, dc.drawable, dc.gc, x, y, buf, len);
}

unsigned int
textw(const char *text) {
	return textnw(text, strlen(text)) + dc.font.height;
}
