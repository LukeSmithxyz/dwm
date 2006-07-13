/*
 * (C)opyright MMIV-MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <stdio.h>
#include <string.h>

#include <X11/Xlocale.h>

#include "wm.h"

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
draw(Bool border, const char *text)
{
	int x, y, w, h;
	unsigned int len;
	static char buf[256];
	XGCValues gcv;
	XRectangle r = { dc.x, dc.y, dc.w, dc.h };

	XSetForeground(dpy, dc.gc, dc.bg);
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
	while(len && (w = textnw(&dc.font, buf, len)) > dc.w - h)
		buf[--len] = 0;

	if(w > dc.w)
		return; /* too long */

	gcv.foreground = dc.fg;
	gcv.background = dc.bg;
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

static unsigned long
xinitcolors(Colormap cmap, const char *colstr)
{
	XColor color;
	XAllocNamedColor(dpy, cmap, colstr, &color, &color);
	return color.pixel;
}

void
initcolors(const char *bg, const char *fg, const char *border)
{
	Colormap cmap = DefaultColormap(dpy, screen);
	dc.bg = xinitcolors(cmap, bg);
	dc.fg = xinitcolors(cmap, fg);
	dc.border = xinitcolors(cmap, border);
}

unsigned int
textnw(Fnt *font, char *text, unsigned int len)
{
	XRectangle r;
	if(font->set) {
		XmbTextExtents(font->set, text, len, NULL, &r);
		return r.width;
	}
	return XTextWidth(font->xfont, text, len);
}

unsigned int
textw(Fnt *font, char *text)
{
	return textnw(font, text, strlen(text));
}

unsigned int
texth(Fnt *font)
{
	return font->height + 4;
}

void
initfont(Fnt *font, const char *fontstr)
{
	char **missing, *def;
	int i, n;

	missing = NULL;
	setlocale(LC_ALL, "");
	if(font->set)
		XFreeFontSet(dpy, font->set);
	font->set = XCreateFontSet(dpy, fontstr, &missing, &n, &def);
	if(missing) {
		while(n--)
			fprintf(stderr, "missing fontset: %s\n", missing[n]);
		XFreeStringList(missing);
		if(font->set) {
			XFreeFontSet(dpy, font->set);
			font->set = NULL;
		}
	}
	if(font->set) {
		XFontSetExtents *font_extents;
		XFontStruct **xfonts;
		char **font_names;

		font->ascent = font->descent = 0;
		font_extents = XExtentsOfFontSet(font->set);
		n = XFontsOfFontSet(font->set, &xfonts, &font_names);
		for(i = 0, font->ascent = 0, font->descent = 0; i < n; i++) {
			if(font->ascent < (*xfonts)->ascent)
				font->ascent = (*xfonts)->ascent;
			if(font->descent < (*xfonts)->descent)
				font->descent = (*xfonts)->descent;
			xfonts++;
		}
	}
	else {
		if(font->xfont)
			XFreeFont(dpy, font->xfont);
		font->xfont = NULL;
		font->xfont = XLoadQueryFont(dpy, fontstr);
		if (!font->xfont)
			font->xfont = XLoadQueryFont(dpy, "fixed");
		if (!font->xfont)
			error("error, cannot init 'fixed' font\n");
		font->ascent = font->xfont->ascent;
		font->descent = font->xfont->descent;
	}
	font->height = font->ascent + font->descent;
}
