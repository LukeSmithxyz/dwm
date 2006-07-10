/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <X11/Xlib.h>
#include <X11/Xlocale.h>

typedef struct Brush Brush;
typedef struct Fnt Fnt;

struct Fnt {
	XFontStruct *xfont;
	XFontSet set;
	int ascent;
	int descent;
	int height;
};

struct Brush {
	GC gc;
	Drawable drawable;
	XRectangle rect;
	Fnt font;
	unsigned long bg;
	unsigned long fg;
	unsigned long border;
};

extern void draw(Display *dpy, Brush *b, Bool border, const char *text);
extern void loadcolors(Display *dpy, int screen, Brush *b,
		const char *bg, const char *fg, const char *bo);
extern void loadfont(Display *dpy, Fnt *font, const char *fontstr);
extern unsigned int textwidth_l(Fnt *font, char *text, unsigned int len);
extern unsigned int textwidth(Fnt *font, char *text);
extern unsigned int labelheight(Fnt *font);
