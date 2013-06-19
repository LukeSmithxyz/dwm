/* See LICENSE file for copyright and license details. */

typedef struct {
	unsigned long rgb;
} Clr;

typedef struct {
	Cursor cursor;
} Cur;

typedef struct {
	int ascent;
	int descent;
	unsigned int h;
	XFontSet set;
	XFontStruct *xfont;
} Fnt;

typedef struct {
	Clr *fg;
	Clr *bg;
	Clr *border;
} ClrScheme;

typedef struct {
	unsigned int w, h;
	Display *dpy;
	int screen;
	Window root;
	Drawable drawable;
	GC gc;
	ClrScheme *scheme;
	Fnt *font;
} Drw;

typedef struct {
	unsigned int w;
	unsigned int h;
} Extnts;

/* Drawable abstraction */
Drw *drw_create(Display *dpy, int screen, Window win, unsigned int w, unsigned int h);
void drw_resize(Drw *drw, unsigned int w, unsigned int h);
void drw_free(Drw *drw);

/* Fnt abstraction */
Fnt *drw_font_create(Display *dpy, const char *fontname);
void drw_font_free(Display *dpy, Fnt *font);
void drw_font_getexts(Fnt *font, const char *text, unsigned int len, Extnts *extnts);
unsigned int drw_font_getexts_width(Fnt *font, const char *text, unsigned int len);

/* Colour abstraction */
Clr *drw_clr_create(Drw *drw, const char *clrname);
void drw_clr_free(Clr *clr);

/* Cursor abstraction */
Cur *drw_cur_create(Drw *drw, int shape);
void drw_cur_free(Drw *drw, Cur *cursor);

/* Drawing context manipulation */
void drw_setfont(Drw *drw, Fnt *font);
void drw_setscheme(Drw *drw, ClrScheme *scheme);

/* Drawing functions */
void drw_rect(Drw *drw, int x, int y, unsigned int w, unsigned int h, int filled, int empty, int invert);
void drw_text(Drw *drw, int x, int y, unsigned int w, unsigned int h, const char *text, int invert);

/* Map functions */
void drw_map(Drw *drw, Window win, int x, int y, unsigned int w, unsigned int h);
