/*
 * (C)opyright MMIV-MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <stdio.h>
#include <string.h>

#include <X11/Xlocale.h>

#include "wm.h"

static void
drawborder(Brush *b)
{
	XPoint points[5];
	XSetLineAttributes(dpy, b->gc, 1, LineSolid, CapButt, JoinMiter);
	XSetForeground(dpy, b->gc, b->border);
	points[0].x = b->x;
	points[0].y = b->y;
	points[1].x = b->w - 1;
	points[1].y = 0;
	points[2].x = 0;
	points[2].y = b->h - 1;
	points[3].x = -(b->w - 1);
	points[3].y = 0;
	points[4].x = 0;
	points[4].y = -(b->h - 1);
	XDrawLines(dpy, b->drawable, b->gc, points, 5, CoordModePrevious);
}

void
draw(Brush *b, Bool border, const char *text)
{
	int x, y, w, h;
	unsigned int len;
	static char buf[256];
	XGCValues gcv;
	XRectangle r = { b->x, b->y, b->w, b->h };

	XSetForeground(dpy, b->gc, b->bg);
	XFillRectangles(dpy, b->drawable, b->gc, &r, 1);

	w = 0;
	if(border)
		drawborder(b);

	if(!text)
		return;

	len = strlen(text);
	if(len >= sizeof(buf))
		len = sizeof(buf) - 1;
	memcpy(buf, text, len);
	buf[len] = 0;

	h = b->font.ascent + b->font.descent;
	y = b->y + (b->h / 2) - (h / 2) + b->font.ascent;
	x = b->x + (h / 2);

	/* shorten text if necessary */
	while(len && (w = textnw(&b->font, buf, len)) > b->w - h)
		buf[--len] = 0;

	if(w > b->w)
		return; /* too long */

	gcv.foreground = b->fg;
	gcv.background = b->bg;
	if(b->font.set) {
		XChangeGC(dpy, b->gc, GCForeground | GCBackground, &gcv);
		XmbDrawImageString(dpy, b->drawable, b->font.set, b->gc,
				x, y, buf, len);
	}
	else {
		gcv.font = b->font.xfont->fid;
		XChangeGC(dpy, b->gc, GCForeground | GCBackground | GCFont, &gcv);
		XDrawImageString(dpy, b->drawable, b->gc, x, y, buf, len);
	}
}

static unsigned long
xloadcolors(Colormap cmap, const char *colstr)
{
	XColor color;
	XAllocNamedColor(dpy, cmap, colstr, &color, &color);
	return color.pixel;
}

void
loadcolors(int scr, Brush *b,
		const char *bg, const char *fg, const char *border)
{
	Colormap cmap = DefaultColormap(dpy, scr);
	b->bg = xloadcolors(cmap, bg);
	b->fg = xloadcolors(cmap, fg);
	b->border = xloadcolors(cmap, border);
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
loadfont(Fnt *font, const char *fontstr)
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
			error("error, cannot load 'fixed' font\n");
		font->ascent = font->xfont->ascent;
		font->descent = font->xfont->descent;
	}
	font->height = font->ascent + font->descent;
}
