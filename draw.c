/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

#include "draw.h"
#include "util.h"

Draw *
draw_create(Display *dpy, int screen, Window win, unsigned int w, unsigned int h) {
	Draw *draw = (Draw *)calloc(1, sizeof(Draw));
	draw->dpy = dpy;
	draw->screen = screen;
	draw->win = win;
	draw->w = w;
	draw->h = h;
	draw->drawable = XCreatePixmap(dpy, win, w, h, DefaultDepth(dpy, screen));
	draw->gc = XCreateGC(dpy, win, 0, NULL);
	XSetLineAttributes(dpy, draw->gc, 1, LineSolid, CapButt, JoinMiter);
	return draw;
}

void
draw_resize(Draw *draw, unsigned int w, unsigned int h) {
	if(!draw)
		return;
	draw->w = w;
	draw->h = h;
	XFreePixmap(draw->dpy, draw->drawable);
	draw->drawable = XCreatePixmap(draw->dpy, draw->win, w, h, DefaultDepth(draw->dpy, draw->screen));
}

void
draw_free(Draw *draw) {
	XFreePixmap(draw->dpy, draw->drawable);
	XFreeGC(draw->dpy, draw->gc);
	free(draw);
}

Fnt *
draw_font_create(Draw *draw, const char *fontname) {
	Fnt *font;
	char *def, **missing;
	int n;

	if(!draw)
		return NULL;
	font = (Fnt *)calloc(1, sizeof(Fnt));
	font->set = XCreateFontSet(draw->dpy, fontname, &missing, &n, &def);
	if(missing) {
		while(n--)
			fprintf(stderr, "draw: missing fontset: %s\n", missing[n]);
		XFreeStringList(missing);
	}
	if(font->set) {
		XFontStruct **xfonts;
		char **font_names;
		XExtentsOfFontSet(font->set);
		n = XFontsOfFontSet(font->set, &xfonts, &font_names);
		while(n--) {
			font->ascent = MAX(font->ascent, (*xfonts)->ascent);
			font->descent = MAX(font->descent,(*xfonts)->descent);
			xfonts++;
		}
	}
	else {
		if(!(font->xfont = XLoadQueryFont(draw->dpy, fontname))
		&& !(font->xfont = XLoadQueryFont(draw->dpy, "fixed")))
			die("error, cannot load font: '%s'\n", fontname);
		font->ascent = font->xfont->ascent;
		font->descent = font->xfont->descent;
	}
	font->h = font->ascent + font->descent;
	return font;
}

void
draw_font_free(Draw *draw, Fnt *font) {
	if(!draw || !font)
		return;
	if(font->set)
		XFreeFontSet(draw->dpy, font->set);
	else
		XFreeFont(draw->dpy, font->xfont);
	free(font);
}

Col *
draw_col_create(Draw *draw, const char *colname) {
	Col *col = (Col *)calloc(1, sizeof(Col));
	Colormap cmap = DefaultColormap(draw->dpy, draw->screen);
	XColor color;

	if(!XAllocNamedColor(draw->dpy, cmap, colname, &color, &color))
		die("error, cannot allocate color '%s'\n", colname);
	col->rgb = color.pixel;
	return col;
}

void
draw_col_free(Draw *draw, Col *col) {
	if(!col)
		return;
	free(col);
}

void
draw_setfont(Draw *draw, Fnt *font) {
	if(!draw)
		return;
	draw->font = font;
}

void
draw_setfg(Draw *draw, Col *col) {
	if(!draw) 
		return;
	draw->fg = col;
}

void
draw_setbg(Draw *draw, Col *col) {
	if(!draw)
		return;
	draw->bg = col;
}

void
draw_rect(Draw *draw, int x, int y, unsigned int w, unsigned int h, Bool filled, Bool empty, Bool invert) {
	int dx;

	if(!draw || !draw->font || !draw->fg || !draw->bg)
		return;
	XSetForeground(draw->dpy, draw->gc, invert ? draw->bg->rgb : draw->fg->rgb);
	dx = (draw->font->ascent + draw->font->descent + 2) / 4;
	if(filled)
		XFillRectangle(draw->dpy, draw->drawable, draw->gc, x+1, y+1, dx+1, dx+1);
	else if(empty)
		XDrawRectangle(draw->dpy, draw->drawable, draw->gc, x+1, y+1, dx, dx);
}

void
draw_text(Draw *draw, int x, int y, unsigned int w, unsigned int h, const char *text, Bool invert) {
	char buf[256];
	int i, tx, ty, len, olen;
	TextExtents tex;

	if(!draw || !draw->fg || !draw->bg)
		return;
	XSetForeground(draw->dpy, draw->gc, invert ? draw->fg->rgb : draw->bg->rgb);
	XFillRectangle(draw->dpy, draw->drawable, draw->gc, x, y, w, h);
	if(!text || !draw->font)
		return;
	olen = strlen(text);
	draw_getextents(draw, text, olen, &tex);
	ty = y + (h / 2) - tex.yOff;
	tx = x + tex.xOff;
	/* shorten text if necessary */
	for(len = MIN(olen, sizeof buf); len && tex.w > w - tex.h; len--)
		draw_getextents(draw, text, len, &tex);
	if(!len)
		return;
	memcpy(buf, text, len);
	if(len < olen)
		for(i = len; i && i > len - 3; buf[--i] = '.');
	XSetForeground(draw->dpy, draw->gc, invert ? draw->bg->rgb : draw->fg->rgb);
	if(draw->font->set)
		XmbDrawString(draw->dpy, draw->drawable, draw->font->set, draw->gc, tx, ty, buf, len);
	else
		XDrawString(draw->dpy, draw->drawable, draw->gc, tx, ty, buf, len);
}

void
draw_map(Draw *draw, int x, int y, unsigned int w, unsigned int h) {
	if(!draw)
		return;
	XCopyArea(draw->dpy, draw->drawable, draw->win, draw->gc, x, y, w, h, x, y);
	XSync(draw->dpy, False);
}


void
draw_getextents(Draw *draw, const char *text, unsigned int len, TextExtents *extents) {
	XRectangle r;

	if(!draw || !draw->font || !text)
		return;
	if(draw->font->set) {
		XmbTextExtents(draw->font->set, text, len, NULL, &r);
		extents->xOff = r.x;
		extents->yOff = r.y;
		extents->w = r.width;
		extents->h = r.height;
	}
	else {
		extents->h = draw->font->ascent + draw->font->descent;
		extents->w = XTextWidth(draw->font->xfont, text, len);
		extents->xOff = extents->h / 2;
		extents->yOff = (extents->h / 2) + draw->font->ascent;
	}
}
