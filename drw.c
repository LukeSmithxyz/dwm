/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

#include "drw.h"
#include "util.h"

Drw *
drw_create(Display *dpy, int screen, Window root, unsigned int w, unsigned int h) {
	Drw *drw = (Drw *)calloc(1, sizeof(Drw));
	if(!drw)
		return NULL;
	drw->dpy = dpy;
	drw->screen = screen;
	drw->root = root;
	drw->w = w;
	drw->h = h;
	drw->drawable = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));
	drw->gc = XCreateGC(dpy, root, 0, NULL);
	XSetLineAttributes(dpy, drw->gc, 1, LineSolid, CapButt, JoinMiter);
	return drw;
}

void
drw_resize(Drw *drw, unsigned int w, unsigned int h) {
	if(!drw)
		return;
	drw->w = w;
	drw->h = h;
	if(drw->drawable != 0)
		XFreePixmap(drw->dpy, drw->drawable);
	drw->drawable = XCreatePixmap(drw->dpy, drw->root, w, h, DefaultDepth(drw->dpy, drw->screen));
}

void
drw_free(Drw *drw) {
	XFreePixmap(drw->dpy, drw->drawable);
	XFreeGC(drw->dpy, drw->gc);
	free(drw);
}

Fnt *
drw_font_create(Display *dpy, const char *fontname) {
	Fnt *font;
	char *def, **missing;
	int n;

	font = (Fnt *)calloc(1, sizeof(Fnt));
	if(!font)
		return NULL;
	font->set = XCreateFontSet(dpy, fontname, &missing, &n, &def);
	if(missing) {
		while(n--)
			fprintf(stderr, "drw: missing fontset: %s\n", missing[n]);
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
		if(!(font->xfont = XLoadQueryFont(dpy, fontname))
		&& !(font->xfont = XLoadQueryFont(dpy, "fixed")))
			die("error, cannot load font: '%s'\n", fontname);
		font->ascent = font->xfont->ascent;
		font->descent = font->xfont->descent;
	}
	font->h = font->ascent + font->descent;
	return font;
}

void
drw_font_free(Display *dpy, Fnt *font) {
	if(!font)
		return;
	if(font->set)
		XFreeFontSet(dpy, font->set);
	else
		XFreeFont(dpy, font->xfont);
	free(font);
}

Clr *
drw_clr_create(Drw *drw, const char *clrname) {
	Clr *clr;
	Colormap cmap;
	XColor color;

	if(!drw)
		return NULL;
	clr = (Clr *)calloc(1, sizeof(Clr));
	if(!clr)
		return NULL;
	cmap = DefaultColormap(drw->dpy, drw->screen);
	if(!XAllocNamedColor(drw->dpy, cmap, clrname, &color, &color))
		die("error, cannot allocate color '%s'\n", clrname);
	clr->rgb = color.pixel;
	return clr;
}

void
drw_clr_free(Clr *clr) {
	if(clr)
		free(clr);
}

void
drw_setfont(Drw *drw, Fnt *font) {
	if(drw)
		drw->font = font;
}

void
drw_setscheme(Drw *drw, ClrScheme *scheme) {
	if(drw && scheme) 
		drw->scheme = scheme;
}

void
drw_rect(Drw *drw, int x, int y, unsigned int w, unsigned int h, int filled, int empty, int invert) {
	int dx;

	if(!drw || !drw->font || !drw->scheme)
		return;
	XSetForeground(drw->dpy, drw->gc, invert ? drw->scheme->bg->rgb : drw->scheme->fg->rgb);
	dx = (drw->font->ascent + drw->font->descent + 2) / 4;
	if(filled)
		XFillRectangle(drw->dpy, drw->drawable, drw->gc, x+1, y+1, dx+1, dx+1);
	else if(empty)
		XDrawRectangle(drw->dpy, drw->drawable, drw->gc, x+1, y+1, dx, dx);
}

void
drw_text(Drw *drw, int x, int y, unsigned int w, unsigned int h, const char *text, int invert) {
	char buf[256];
	int i, tx, ty, th, len, olen;
	Extnts tex;

	if(!drw || !drw->scheme)
		return;
	XSetForeground(drw->dpy, drw->gc, invert ? drw->scheme->fg->rgb : drw->scheme->bg->rgb);
	XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w, h);
	if(!text || !drw->font)
		return;
	olen = strlen(text);
	drw_font_getexts(drw->font, text, olen, &tex);
	th = drw->font->ascent + drw->font->descent;
	ty = y + (h / 2) - (th / 2) + drw->font->ascent;
	tx = x + (h / 2);
	/* shorten text if necessary */
	for(len = MIN(olen, sizeof buf); len && (tex.w > w - tex.h || w < tex.h); len--)
		drw_font_getexts(drw->font, text, len, &tex);
	if(!len)
		return;
	memcpy(buf, text, len);
	if(len < olen)
		for(i = len; i && i > len - 3; buf[--i] = '.');
	XSetForeground(drw->dpy, drw->gc, invert ? drw->scheme->bg->rgb : drw->scheme->fg->rgb);
	if(drw->font->set)
		XmbDrawString(drw->dpy, drw->drawable, drw->font->set, drw->gc, tx, ty, buf, len);
	else
		XDrawString(drw->dpy, drw->drawable, drw->gc, tx, ty, buf, len);
}

void
drw_map(Drw *drw, Window win, int x, int y, unsigned int w, unsigned int h) {
	if(!drw)
		return;
	XCopyArea(drw->dpy, drw->drawable, win, drw->gc, x, y, w, h, x, y);
	XSync(drw->dpy, False);
}


void
drw_font_getexts(Fnt *font, const char *text, unsigned int len, Extnts *tex) {
	XRectangle r;

	if(!font || !text)
		return;
	if(font->set) {
		XmbTextExtents(font->set, text, len, NULL, &r);
		tex->w = r.width;
		tex->h = r.height;
	}
	else {
		tex->h = font->ascent + font->descent;
		tex->w = XTextWidth(font->xfont, text, len);
	}
}

unsigned int
drw_font_getexts_width(Fnt *font, const char *text, unsigned int len) {
	Extnts tex;

	if(!font)
		return -1;
	drw_font_getexts(font, text, len, &tex);
	return tex.w;
}

Cur *
drw_cur_create(Drw *drw, int shape) {
	Cur *cur = (Cur *)calloc(1, sizeof(Cur));

	if(!drw || !cur)
		return NULL;
	cur->cursor = XCreateFontCursor(drw->dpy, shape);
	return cur;
}

void
drw_cur_free(Drw *drw, Cur *cursor) {
	if(!drw || !cursor)
		return;
	XFreeCursor(drw->dpy, cursor->cursor);
	free(cursor);
}
