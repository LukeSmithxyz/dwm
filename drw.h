/* See LICENSE file for copyright and license details. */

typedef struct {
	unsigned long rgb;
} Clr;

typedef struct {
	int ascent;
	int descent;
	unsigned int h;
	XFontSet set;
	XFontStruct *xfont;
} Fnt;

typedef struct {
	unsigned int w, h;
	Display *dpy;
	int screen;
	Window win;
	Drawable drwable;
	GC gc;
	Clr *fg;
	Clr *bg;
	Fnt *font;
} Drw;

typedef struct {
	unsigned int w;
	unsigned int h;
	int xOff;
	int yOff;
} Extnts;

/* Drawable abstraction */
Drw *drw_create(Display *dpy, int screen, Window win, unsigned int w, unsigned int h);
void drw_resize(Drw *drw, unsigned int w, unsigned int h);
void drw_free(Drw *drw);

/* Fnt abstraction */
Fnt *drw_font_create(Drw *drw, const char *fontname);
void drw_font_free(Drw *drw, Fnt *font);

/* Clrour abstraction */
Clr *drw_clr_create(Drw *drw, const char *clrname);
void drw_clr_free(Drw *drw, Clr *clr);

/* Drawing context manipulation */
void drw_setfont(Drw *drw, Fnt *font);
void drw_setfg(Drw *drw, Clr *clr);
void drw_setbg(Drw *drw, Clr *clr);

/* Drawing functions */
void drw_rect(Drw *drw, int x, int y, unsigned int w, unsigned int h, Bool filled, Bool empty, Bool invert);
void drw_text(Drw *drw, int x, int y, unsigned int w, unsigned int h, const char *text, Bool invert);

/* Map functions */
void drw_map(Drw *drw, int x, int y, unsigned int w, unsigned int h);

/* Text functions */
void drw_getexts(Drw *drw, const char *text, unsigned int len, Extnts *extnts);

