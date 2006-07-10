/*
 * (C)opyright MMIV-MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <stdio.h>
#include <string.h>

#include "draw.h"
#include "util.h"

static void
drawborder(Display *dpy, Brush *b)
{
	XPoint points[5];
	XSetLineAttributes(dpy, b->gc, 1, LineSolid, CapButt, JoinMiter);
	XSetForeground(dpy, b->gc, b->color.border);
	points[0].x = b->rect.x;
	points[0].y = b->rect.y;
	points[1].x = b->rect.width - 1;
	points[1].y = 0;
	points[2].x = 0;
	points[2].y = b->rect.height - 1;
	points[3].x = -(b->rect.width - 1);
	points[3].y = 0;
	points[4].x = 0;
	points[4].y = -(b->rect.height - 1);
	XDrawLines(dpy, b->drawable, b->gc, points, 5, CoordModePrevious);
}

void
draw(Display *dpy, Brush *b)
{
	unsigned int x, y, w, h, len;
	static char buf[256];
	XGCValues gcv;

	XSetForeground(dpy, b->gc, b->color.bg);
	XFillRectangles(dpy, b->drawable, b->gc, &b->rect, 1);

	if(b->border)
		drawborder(dpy, b);

	if(!b->text)
		return;

	len = strlen(b->text);
	if(len >= sizeof(buf))
		len = sizeof(buf) - 1;
	memcpy(buf, b->text, len);
	buf[len] = 0;

	h = b->font->ascent + b->font->descent;
	y = b->rect.y + (b->rect.height / 2) - (h / 2) + b->font->ascent;
	x = b->rect.x + (h / 2);

	/* shorten text if necessary */
	while(len && (w = textwidth_l(b->font, buf, len)) > b->rect.width - h)
		buf[--len] = 0;

	if(w > b->rect.width)
		return; /* too long */

	gcv.foreground = b->color.fg;
	gcv.background = b->color.bg;
	if(b->font->set) {
		XChangeGC(dpy, b->gc, GCForeground | GCBackground, &gcv);
		XmbDrawImageString(dpy, b->drawable, b->font->set, b->gc,
				x, y, buf, len);
	}
	else {
		gcv.font = b->font->xfont->fid;
		XChangeGC(dpy, b->gc, GCForeground | GCBackground | GCFont, &gcv);
		XDrawImageString(dpy, b->drawable, b->gc, x, y, buf, len);
	}
}

static unsigned long
xloadcolor(Display *dpy, Colormap cmap, const char *colstr)
{
	XColor color;
	XAllocNamedColor(dpy, cmap, colstr, &color, &color);
	return color.pixel;
}

void
loadcolor(Display *dpy, int screen, Color *c,
		const char *bg, const char *fg, const char *border)
{
	Colormap cmap = DefaultColormap(dpy, screen);
	c->bg = xloadcolor(dpy, cmap, bg);
	c->fg = xloadcolor(dpy, cmap, fg);
	c->border = xloadcolor(dpy, cmap, border);
}

unsigned int
textwidth_l(Fnt *font, char *text, unsigned int len)
{
	if(font->set) {
		XRectangle r;
		XmbTextExtents(font->set, text, len, 0, &r);
		return r.width;
	}
	return XTextWidth(font->xfont, text, len);
}

unsigned int
textwidth(Fnt *font, char *text)
{
	return textwidth_l(font, text, strlen(text));
}

void
loadfont(Display *dpy, Fnt *font, const char *fontstr)
{
	char **missing, *def;
	int n;

	missing = 0;
	def = "?";
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
			font->set = 0;
		}
	}
	if(font->set) {
		XFontSetExtents *font_extents;
		XFontStruct **xfonts;
		char **font_names;
		unsigned int i;

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
		font->xfont = 0;
		font->xfont = XLoadQueryFont(dpy, fontstr);
		if (!font->xfont)
			font->xfont = XLoadQueryFont(dpy, "fixed");
		if (!font->xfont)
			error("error, cannot load 'fixed' font\n");
		font->ascent = font->xfont->ascent;
		font->descent = font->xfont->descent;
	}
}
