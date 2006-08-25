/*
 * (C)opyright MMIV-MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"
#include <stdio.h>
#include <string.h>
#include <X11/Xlocale.h>

/* static */

static unsigned int
textnw(const char *text, unsigned int len)
{
	XRectangle r;

	if(dc.font.set) {
		XmbTextExtents(dc.font.set, text, len, NULL, &r);
		return r.width;
	}
	return XTextWidth(dc.font.xfont, text, len);
}

static void
drawtext(const char *text, unsigned long col[ColLast], Bool highlight)
{
	int x, y, w, h;
	static char buf[256];
	unsigned int len, olen;
	XGCValues gcv;
	XRectangle r = { dc.x, dc.y, dc.w, dc.h };

	XSetForeground(dpy, dc.gc, col[ColBG]);
	XFillRectangles(dpy, dc.drawable, dc.gc, &r, 1);

	if(!text)
		return;

	w = 0;
	olen = len = strlen(text);
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
	gcv.foreground = col[ColFG];
	if(dc.font.set) {
		XChangeGC(dpy, dc.gc, GCForeground, &gcv);
		XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, x, y, buf, len);
	}
	else {
		gcv.font = dc.font.xfont->fid;
		XChangeGC(dpy, dc.gc, GCForeground | GCFont, &gcv);
		XDrawString(dpy, dc.drawable, dc.gc, x, y, buf, len);
	}
	if(highlight) {
		r.x = dc.x + 2;
		r.y = dc.y + 2;
		r.width = r.height = 3;
		XFillRectangles(dpy, dc.drawable, dc.gc, &r, 1);
	}
}

/* extern */

void
drawall()
{
	Client *c;

	for(c = clients; c; c = getnext(c->next))
		drawtitle(c);
	drawstatus();
}

void
drawstatus()
{
	int i, x;

	dc.x = dc.y = 0;
	dc.w = bw;

	drawtext(NULL, dc.status, False);
	for(i = 0; i < ntags; i++) {
		dc.w = textw(tags[i]);
		if(seltag[i])
			drawtext(tags[i], dc.sel, sel && sel->tags[i]);
		else
			drawtext(tags[i], dc.norm, sel && sel->tags[i]);
		dc.x += dc.w;
	}

	dc.w = bmw;
	drawtext(arrange == dotile ? TILESYMBOL : FLOATSYMBOL, dc.status, False);

	x = dc.x + dc.w;
	dc.w = textw(stext);
	dc.x = bx + bw - dc.w;
	if(dc.x < x) {
		dc.x = x;
		dc.w = bw - x;
	}
	drawtext(stext, dc.status, False);

	if(sel && ((dc.w = dc.x - x) > bh)) {
		dc.x = x;
		drawtext(sel->name, dc.sel, False);
	}
	XCopyArea(dpy, dc.drawable, barwin, dc.gc, 0, 0, bw, bh, 0, 0);
	XSync(dpy, False);
}

void
drawtitle(Client *c)
{
	int i;

	if(c == sel && issel) {
		drawstatus();
		XUnmapWindow(dpy, c->twin);
		XSetWindowBorder(dpy, c->win, dc.sel[ColBG]);
		return;
	}

	XSetWindowBorder(dpy, c->win, dc.norm[ColBG]);
	XMapWindow(dpy, c->twin);
	dc.x = dc.y = 0;
	dc.w = c->tw;
	drawtext(c->name, dc.norm, False);
	XCopyArea(dpy, dc.drawable, c->twin, dc.gc, 0, 0, c->tw, c->th, 0, 0);
	XSync(dpy, False);
}

unsigned long
getcolor(const char *colstr)
{
	Colormap cmap = DefaultColormap(dpy, screen);
	XColor color;

	XAllocNamedColor(dpy, cmap, colstr, &color, &color);
	return color.pixel;
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
			eprint("error, cannot init 'fixed' font\n");
		dc.font.ascent = dc.font.xfont->ascent;
		dc.font.descent = dc.font.xfont->descent;
	}
	dc.font.height = dc.font.ascent + dc.font.descent;
}

unsigned int
textw(const char *text)
{
	return textnw(text, strlen(text)) + dc.font.height;
}
