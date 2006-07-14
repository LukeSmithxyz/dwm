/*
 * (C)opyright MMIV-MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <stdio.h>
#include <string.h>

#include <X11/Xlocale.h>

#include "dwm.h"

void
drawstatus()
{
	int i;

	dc.x = dc.y = 0;
	dc.w = bw;
	drawtext(NULL, False, False);

	if(arrange == floating) {
		dc.w = textw("~");
		drawtext("~", False, False);
	}
	else
		dc.w = 0;
	for(i = 0; i < TLast; i++) {
		dc.x += dc.w;
		dc.w = textw(tags[i]);
		drawtext(tags[i], i == tsel, True);
	}
	if(sel) {
		dc.x += dc.w;
		dc.w = textw(sel->name);
		drawtext(sel->name, True, True);
	}
	dc.w = textw(stext);
	dc.x = bx + bw - dc.w;
	drawtext(stext, False, False);

	XCopyArea(dpy, dc.drawable, barwin, dc.gc, 0, 0, bw, bh, 0, 0);
	XFlush(dpy);
}

void
drawtitle(Client *c)
{
	int i;
	if(c == sel) {
		drawstatus();
		XUnmapWindow(dpy, c->title);
		XSetWindowBorder(dpy, c->win, dc.fg);
		return;
	}

	XSetWindowBorder(dpy, c->win, dc.bg);
	XMapWindow(dpy, c->title);

	dc.x = dc.y = 0;

	dc.w = 0;
	for(i = 0; i < TLast; i++) {
		if(c->tags[i]) {
			dc.x += dc.w;
			dc.w = textw(c->tags[i]);
			drawtext(c->tags[i], False, True);
		}
	}
	dc.x += dc.w;
	dc.w = textw(c->name);
	drawtext(c->name, False, True);
	XCopyArea(dpy, dc.drawable, c->title, dc.gc,
			0, 0, c->tw, c->th, 0, 0);
	XFlush(dpy);
}

static void
drawborder(void)
{
	XPoint points[5];
	XSetLineAttributes(dpy, dc.gc, 1, LineSolid, CapButt, JoinMiter);
	XSetForeground(dpy, dc.gc, dc.border);
	points[0].x = dc.x;
	points[0].y = dc.y;
	points[1].x = dc.w - 1;
	points[1].y = 0;
	points[2].x = 0;
	points[2].y = dc.h - 1;
	points[3].x = -(dc.w - 1);
	points[3].y = 0;
	points[4].x = 0;
	points[4].y = -(dc.h - 1);
	XDrawLines(dpy, dc.drawable, dc.gc, points, 5, CoordModePrevious);
}

void
drawtext(const char *text, Bool invert, Bool border)
{
	int x, y, w, h;
	unsigned int len;
	static char buf[256];
	XGCValues gcv;
	XRectangle r = { dc.x, dc.y, dc.w, dc.h };

	XSetForeground(dpy, dc.gc, invert ? dc.fg : dc.bg);
	XFillRectangles(dpy, dc.drawable, dc.gc, &r, 1);

	w = 0;
	if(border)
		drawborder();

	if(!text)
		return;

	len = strlen(text);
	if(len >= sizeof(buf))
		len = sizeof(buf) - 1;
	memcpy(buf, text, len);
	buf[len] = 0;

	h = dc.font.ascent + dc.font.descent;
	y = dc.y + (dc.h / 2) - (h / 2) + dc.font.ascent;
	x = dc.x + (h / 2);

	/* shorten text if necessary */
	while(len && (w = textnw(buf, len)) > dc.w - h)
		buf[--len] = 0;

	if(w > dc.w)
		return; /* too long */

	gcv.foreground = invert ? dc.bg : dc.fg;
	gcv.background = invert ? dc.fg : dc.bg;
	if(dc.font.set) {
		XChangeGC(dpy, dc.gc, GCForeground | GCBackground, &gcv);
		XmbDrawImageString(dpy, dc.drawable, dc.font.set, dc.gc,
				x, y, buf, len);
	}
	else {
		gcv.font = dc.font.xfont->fid;
		XChangeGC(dpy, dc.gc, GCForeground | GCBackground | GCFont, &gcv);
		XDrawImageString(dpy, dc.drawable, dc.gc, x, y, buf, len);
	}
}

unsigned long
getcolor(const char *colstr)
{
	XColor color;
	Colormap cmap = DefaultColormap(dpy, screen);

	XAllocNamedColor(dpy, cmap, colstr, &color, &color);
	return color.pixel;
}

unsigned int
textnw(char *text, unsigned int len)
{
	XRectangle r;
	if(dc.font.set) {
		XmbTextExtents(dc.font.set, text, len, NULL, &r);
		return r.width;
	}
	return XTextWidth(dc.font.xfont, text, len);
}

unsigned int
textw(char *text)
{
	return textnw(text, strlen(text)) + dc.font.height;
}

void
setfont(const char *fontstr)
{
	char **missing, *def;
	int i, n;

	missing = NULL;
	setlocale(LC_ALL, "");
	if(dc.font.set)
		XFreeFontSet(dpy, dc.font.set);
	dc.font.set = XCreateFontSet(dpy, fontstr, &missing, &n, &def);
	if(missing) {
		while(n--)
			fprintf(stderr, "missing fontset: %s\n", missing[n]);
		XFreeStringList(missing);
		if(dc.font.set) {
			XFreeFontSet(dpy, dc.font.set);
			dc.font.set = NULL;
		}
	}
	if(dc.font.set) {
		XFontSetExtents *font_extents;
		XFontStruct **xfonts;
		char **font_names;

		dc.font.ascent = dc.font.descent = 0;
		font_extents = XExtentsOfFontSet(dc.font.set);
		n = XFontsOfFontSet(dc.font.set, &xfonts, &font_names);
		for(i = 0, dc.font.ascent = 0, dc.font.descent = 0; i < n; i++) {
			if(dc.font.ascent < (*xfonts)->ascent)
				dc.font.ascent = (*xfonts)->ascent;
			if(dc.font.descent < (*xfonts)->descent)
				dc.font.descent = (*xfonts)->descent;
			xfonts++;
		}
	}
	else {
		if(dc.font.xfont)
			XFreeFont(dpy, dc.font.xfont);
		dc.font.xfont = NULL;
		dc.font.xfont = XLoadQueryFont(dpy, fontstr);
		if (!dc.font.xfont)
			dc.font.xfont = XLoadQueryFont(dpy, "fixed");
		if (!dc.font.xfont)
			error("error, cannot init 'fixed' font\n");
		dc.font.ascent = dc.font.xfont->ascent;
		dc.font.descent = dc.font.xfont->descent;
	}
	dc.font.height = dc.font.ascent + dc.font.descent;
}
