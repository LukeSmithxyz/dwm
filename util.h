/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <X11/Xlib.h>
#include <X11/Xlocale.h>

typedef struct Brush Brush;
typedef struct Color Color;
typedef struct Fnt Fnt;

struct Color {
	unsigned long bg;
	unsigned long fg;
	unsigned long border;
};

struct Fnt {
	XFontStruct *xfont;
	XFontSet set;
	int ascent;
	int descent;
};

struct Brush {
	GC gc;
	Drawable drawable;
	XRectangle rect;
	Bool border;
	Fnt *font;
	Color color;
	const char *text;
};

extern void draw(Display *dpy, Brush *b);
extern void loadcolor(Display *dpy, int screen, Color *c,
		const char *bg, const char *fg, const char *bo);
extern unsigned int textwidth_l(Fnt *font, char *text, unsigned int len);
extern unsigned int textwidth(Fnt *font, char *text);
extern void loadfont(Display *dpy, Fnt *font, const char *fontstr);
