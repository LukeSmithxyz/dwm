/* See LICENSE file for copyright and license details. */
#include <stdlib.h>
#include <X11/Xlib.h>

#include "draw.h"

Draw *
draw_create(Display *dpy, Window win, unsigned int w, unsigned int h) {
	Draw *draw = (Draw *)calloc(1, sizeof(Draw));
	draw->w = w;
	draw->h = h;
	/* TODO: drawable creation */
	/* TODO: gc allocation */
	return draw;
}

void
draw_resize(Draw *draw, unsigned int w, unsigned int h) {
	if(!draw)
		return;
	draw->w = w;
	draw->h = h;
	/* TODO: resize drawable */
}

void
draw_free(Draw *draw) {
	/* TODO: deallocate DDCs */
	/* TODO: deallocate drawable */
	free(draw);
}

DDC *
dc_create(Draw *draw) {
	DDC *dc = (DDC *)calloc(1, sizeof(DDC));
	dc->draw = draw;
	dc->next = draw->dc;
	draw->dc = dc;
	return dc;
}

void
dc_free(DDC *dc) {
	DDC **tdc;

	if(!dc)
		return;
	/* remove from dc list */
	for(tdc = &dc->draw->dc; *tdc && *tdc != dc; tdc = &(*tdc)->next);
	*tdc = dc->next;
	/* TODO: deallocate any resources of this dc, if needed */
	free(dc);
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
dc_setfont(DDC *dc, Fnt *font) {
	if(!dc || !font)
		return;
	dc->font = font;
}

void
dc_setfg(DDC *dc, Col *col) {
	if(!dc || !col) 
		return;
	dc->fg = col;
}

void
dc_setbg(DDC *dc, Col *col) {
	if(!dc || !col)
		return;
	dc->bg = col;
}

void
dc_setfill(DDC *dc, Bool fill) {
	if(!dc)
		return;
	dc->fill = fill;
}

void
dc_drawrect(DDC *dc, int x, int y, unsigned int w, unsigned int h) {
	if(!dc)
		return;
	/* TODO: draw the rectangle */
}

void
dc_drawtext(DDC *dc, int x, int y, const char *text) {
	if(!dc)
		return;
	/* TODO: draw the text */
}

void
dc_map(DDC *dc, int x, int y, unsigned int w, unsigned int h) {
	if(!dc)
		return;
	/* TODO: map the dc contents in the region */
}

void
dc_getextents(DDC *dc, const char *text, TextExtents *extents) {
	if(!dc || !extents)
		return;
	/* TODO: get extents */
}
