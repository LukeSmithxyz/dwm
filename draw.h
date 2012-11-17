/* See LICENSE file for copyright and license details. */

/* X11 types - begin */
struct _XDraw {
	unsigned int w, h;
	Display *dpy;
	Drawable drawable;
	GC gc;
};
typedef struct _XDraw Draw;

struct _XCol {
	unsigned long rgb;
};
typedef struct _XCol Col;

struct _XFont {
	int ascent;
	int descent;
	unsigned int h, w;
	XFontSet set;
	XFontStruct *xfont;
};
typedef struct _XFont Fnt;
/* X11 types - end */

typedef struct {
	Draw *draw;
	Col *fg;
	Col *bg;
	Fnt *font;
	Bool fill;
} DDC;

typedef struct {
	unsigned int w;
	unsigned int h;
	int x;
	int y;
	int xOff;
	int yOff;
} TextExtents;

/* Drawable abstraction */
Draw *draw_create(Display *dpy, Window win, unsigned int w, unsigned int h);
void draw_resize(Draw *draw, unsigned int w, unsigned int h);
void draw_free(Draw *draw);

/* Drawing context abstraction */
DDC *dc_create(Draw *draw);
void dc_free(DDC *dc);

/* Fnt abstraction */
Fnt *font_create(const char *fontname);
void font_free(Fnt *font);

/* Colour abstraction */
Col *col_create(const char *colname);
void col_free(Col *col);

/* Drawing context manipulation */
void dc_setfont(DDC *dc, Fnt *font);
void dc_setfg(DDC *dc, Col col);
void dc_setbg(DDC *dc, Col col);
void dc_setfill(DDC *dc, Bool fill);

/* Drawing functions */
void dc_drawrect(DDC *dc, int x, int y, unsigned int w, unsigned int h);
void dc_drawtext(DDC *dc, int x, int y, const char *text);

/* Map functions */
void dc_map(DDC *dc, int x, int y, unsigned int w, unsigned int h);

/* Text functions */
void dc_getextents(DDC *dc, const char *text, TextExtents *extents);

