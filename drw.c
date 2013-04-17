/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

#include "drw.h"
#include "util.h"

Drw *
drw_create(Display *dpy, int screen, Window win, unsigned int w, unsigned int h) {
	Drw *drw = (Drw *)calloc(1, sizeof(Drw));
	drw->dpy = dpy;
	drw->screen = screen;
	drw->win = win;
	drw->w = w;
	drw->h = h;
	drw->drwable = XCreatePixmap(dpy, win, w, h, DefaultDepth(dpy, screen));
	drw->gc = XCreateGC(dpy, win, 0, NULL);
	XSetLineAttributes(dpy, drw->gc, 1, LineSolid, CapButt, JoinMiter);
	return drw;
}

void
drw_resize(Drw *drw, unsigned int w, unsigned int h) {
	if(!drw)
		return;
	drw->w = w;
	drw->h = h;
	XFreePixmap(drw->dpy, drw->drwable);
	drw->drwable = XCreatePixmap(drw->dpy, drw->win, w, h, DefaultDepth(drw->dpy, drw->screen));
}

void
drw_free(Drw *drw) {
	XFreePixmap(drw->dpy, drw->drwable);
	XFreeGC(drw->dpy, drw->gc);
	free(drw);
}

Fnt *
drw_font_create(Drw *drw, const char *fontname) {
	Fnt *font;
	char *def, **missing;
	int n;

	if(!drw)
		return NULL;
	font = (Fnt *)calloc(1, sizeof(Fnt));
	font->set = XCreateFontSet(drw->dpy, fontname, &missing, &n, &def);
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
		if(!(font->xfont = XLoadQueryFont(drw->dpy, fontname))
		&& !(font->xfont = XLoadQueryFont(drw->dpy, "fixed")))
			die("error, cannot load font: '%s'\n", fontname);
		font->ascent = font->xfont->ascent;
		font->descent = font->xfont->descent;
	}
	font->h = font->ascent + font->descent;
	return font;
}

void
drw_font_free(Drw *drw, Fnt *font) {
	if(!drw || !font)
		return;
	if(font->set)
		XFreeFontSet(drw->dpy, font->set);
	else
		XFreeFont(drw->dpy, font->xfont);
	free(font);
}

Clr *
drw_clr_create(Drw *drw, const char *clrname) {
	Clr *clr = (Clr *)calloc(1, sizeof(Clr));
	Colormap cmap = DefaultColormap(drw->dpy, drw->screen);
	XColor color;

	if(!XAllocNamedColor(drw->dpy, cmap, clrname, &color, &color))
		die("error, cannot allocate color '%s'\n", clrname);
	clr->rgb = color.pixel;
	return clr;
}

void
drw_clr_free(Drw *drw, Clr *clr) {
	if(!clr)
		return;
	free(clr);
}

void
drw_setfont(Drw *drw, Fnt *font) {
	if(!drw)
		return;
	drw->font = font;
}

void
drw_setfg(Drw *drw, Clr *clr) {
	if(!drw) 
		return;
	drw->fg = clr;
}

void
drw_setbg(Drw *drw, Clr *clr) {
	if(!drw)
		return;
	drw->bg = clr;
}

void
drw_rect(Drw *drw, int x, int y, unsigned int w, unsigned int h, Bool filled, Bool empty, Bool invert) {
	int dx;

	if(!drw || !drw->font || !drw->fg || !drw->bg)
		return;
	XSetForeground(drw->dpy, drw->gc, invert ? drw->bg->rgb : drw->fg->rgb);
	dx = (drw->font->ascent + drw->font->descent + 2) / 4;
	if(filled)
		XFillRectangle(drw->dpy, drw->drwable, drw->gc, x+1, y+1, dx+1, dx+1);
	else if(empty)
		XDrawRectangle(drw->dpy, drw->drwable, drw->gc, x+1, y+1, dx, dx);
}

void
drw_text(Drw *drw, int x, int y, unsigned int w, unsigned int h, const char *text, Bool invert) {
	char buf[256];
	int i, tx, ty, len, olen;
	Extnts tex;

	if(!drw || !drw->fg || !drw->bg)
		return;
	XSetForeground(drw->dpy, drw->gc, invert ? drw->fg->rgb : drw->bg->rgb);
	XFillRectangle(drw->dpy, drw->drwable, drw->gc, x, y, w, h);
	if(!text || !drw->font)
		return;
	olen = strlen(text);
	drw_getexts(drw, text, olen, &tex);
	ty = y + (h / 2) - tex.yOff;
	tx = x + tex.xOff;
	/* shorten text if necessary */
	for(len = MIN(olen, sizeof buf); len && tex.w > w - tex.h; len--)
		drw_getexts(drw, text, len, &tex);
	if(!len)
		return;
	memcpy(buf, text, len);
	if(len < olen)
		for(i = len; i && i > len - 3; buf[--i] = '.');
	XSetForeground(drw->dpy, drw->gc, invert ? drw->bg->rgb : drw->fg->rgb);
	if(drw->font->set)
		XmbDrawString(drw->dpy, drw->drwable, drw->font->set, drw->gc, tx, ty, buf, len);
	else
		XDrawString(drw->dpy, drw->drwable, drw->gc, tx, ty, buf, len);
}

void
drw_map(Drw *drw, int x, int y, unsigned int w, unsigned int h) {
	if(!drw)
		return;
	XCopyArea(drw->dpy, drw->drwable, drw->win, drw->gc, x, y, w, h, x, y);
	XSync(drw->dpy, False);
}


void
drw_getexts(Drw *drw, const char *text, unsigned int len, Extnts *tex) {
	XRectangle r;

	if(!drw || !drw->font || !text)
		return;
	if(drw->font->set) {
		XmbTextExtents(drw->font->set, text, len, NULL, &r);
		tex->xOff = r.x;
		tex->yOff = r.y;
		tex->w = r.width;
		tex->h = r.height;
	}
	else {
		tex->h = drw->font->ascent + drw->font->descent;
		tex->w = XTextWidth(drw->font->xfont, text, len);
		tex->xOff = tex->h / 2;
		tex->yOff = (tex->h / 2) + drw->font->ascent;
	}
}
