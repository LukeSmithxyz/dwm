/* See LICENSE file for copyright and license details. */
#include <stdlib.h>
#include <X11/Xlib.h>

#include "draw.h"

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
font_create(const char *fontname) {
	Fnt *font = (Fnt *)calloc(1, sizeof(Fnt));
	/* TODO: allocate actual font */
	return font;
}

void
font_free(Fnt *font) {
	if(!font)
		return;
	/* TODO: deallocate any font resources */
	free(font);
}

Col *
col_create(const char *colname) {
	Col *col = (Col *)calloc(1, sizeof(Col));
	/* TODO: allocate color */
	return col;
}

void
col_free(Col *col) {
	if(!col)
		return;
	/* TODO: deallocate any color resource */
	free(col);
}

void
draw_setfont(Draw *draw, Fnt *font) {
	if(!draw || !font)
		return;
	draw->font = font;
}

void
draw_setfg(Draw *draw, Col *col) {
	if(!draw || !col) 
		return;
	draw->fg = col;
}

void
draw_setbg(Draw *draw, Col *col) {
	if(!draw || !col)
		return;
	draw->bg = col;
}

void
draw_rect(Draw *draw, int x, int y, unsigned int w, unsigned int h) {
	if(!draw)
		return;
	/* TODO: draw the rectangle */
}

void
draw_text(Draw *draw, int x, int y, const char *text) {
	if(!draw)
		return;
	/* TODO: draw the text */
}

void
draw_map(Draw *draw, int x, int y, unsigned int w, unsigned int h) {
	if(!draw)
		return;
	/* TODO: map the draw contents in the region */
}

void
draw_getextents(Draw *draw, const char *text, TextExtents *extents) {
	if(!draw || !extents)
		return;
	/* TODO: get extents */
}
