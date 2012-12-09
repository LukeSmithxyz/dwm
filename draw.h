/* See LICENSE file for copyright and license details. */

struct _XCol {
	unsigned long rgb;
};
typedef struct _XCol Col;

struct _XFont {
	int ascent;
	int descent;
	unsigned int h;
	XFontSet set;
	XFontStruct *xfont;
};
typedef struct _XFont Fnt;

typedef struct _XDraw Draw;
struct _XDraw {
	unsigned int w, h;
	Display *dpy;
	int screen;
	Window win;
	Drawable drawable;
	GC gc;
	Col *fg;
	Col *bg;
	Fnt *font;
};

typedef struct {
	unsigned int w;
	unsigned int h;
	int xOff;
	int yOff;
} TextExtents;

/* Drawable abstraction */
Draw *draw_create(Display *dpy, int screen, Window win, unsigned int w, unsigned int h);
void draw_resize(Draw *draw, unsigned int w, unsigned int h);
void draw_free(Draw *draw);

/* Fnt abstraction */
Fnt *draw_font_create(Draw *draw, const char *fontname);
void draw_font_free(Draw *draw, Fnt *font);

/* Colour abstraction */
Col *draw_col_create(Draw *draw, const char *colname);
void draw_col_free(Draw *draw, Col *col);

/* Drawing context manipulation */
void draw_setfont(Draw *draw, Fnt *font);
void draw_setfg(Draw *draw, Col *col);
void draw_setbg(Draw *draw, Col *col);

/* Drawing functions */
void draw_rect(Draw *draw, int x, int y, unsigned int w, unsigned int h, Bool filled, Bool empty, Bool invert);
void draw_text(Draw *draw, int x, int y, unsigned int w, unsigned int h, const char *text, Bool invert);

/* Map functions */
void draw_map(Draw *draw, int x, int y, unsigned int w, unsigned int h);

/* Text functions */
void draw_getextents(Draw *draw, const char *text, unsigned int len, TextExtents *extents);

