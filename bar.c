/* See LICENSE file for copyright and license details. */
#include "dwm.h"
#include <string.h>
#include <stdio.h>

/* static */

static void
drawsquare(Bool filled, Bool empty, unsigned long col[ColLast]) {
	int x;
	XGCValues gcv;
	XRectangle r = { dc.x, dc.y, dc.w, dc.h };

	gcv.foreground = col[ColFG];
	XChangeGC(dpy, dc.gc, GCForeground, &gcv);
	x = (dc.font.ascent + dc.font.descent + 2) / 4;
	r.x = dc.x + 1;
	r.y = dc.y + 1;
	if(filled) {
		r.width = r.height = x + 1;
		XFillRectangles(dpy, dc.drawable, dc.gc, &r, 1);
	}
	else if(empty) {
		r.width = r.height = x;
		XDrawRectangles(dpy, dc.drawable, dc.gc, &r, 1);
	}
}

static unsigned long
initcolor(const char *colstr) {
	Colormap cmap = DefaultColormap(dpy, screen);
	XColor color;

	if(!XAllocNamedColor(dpy, cmap, colstr, &color, &color))
		eprint("error, cannot allocate color '%s'\n", colstr);
	return color.pixel;
}

static void
initfont(const char *fontstr) {
	char *def, **missing;
	int i, n;

	missing = NULL;
	if(dc.font.set)
		XFreeFontSet(dpy, dc.font.set);
	dc.font.set = XCreateFontSet(dpy, fontstr, &missing, &n, &def);
	if(missing) {
		while(n--)
			fprintf(stderr, "dwm: missing fontset: %s\n", missing[n]);
		XFreeStringList(missing);
	}
	if(dc.font.set) {
		XFontSetExtents *font_extents;
		XFontStruct **xfonts;
		char **font_names;
		dc.font.ascent = dc.font.descent = 0;
		font_extents = XExtentsOfFontSet(dc.font.set);
		n = XFontsOfFontSet(dc.font.set, &xfonts, &font_names);
		for(i = 0, dc.font.ascent = 0, dc.font.descent = 0; i < n; i++) {
			if(dc.font.ascent < (*xfonts)->ascent)
				dc.font.ascent = (*xfonts)->ascent;
			if(dc.font.descent < (*xfonts)->descent)
				dc.font.descent = (*xfonts)->descent;
			xfonts++;
		}
	}
	else {
		if(dc.font.xfont)
			XFreeFont(dpy, dc.font.xfont);
		dc.font.xfont = NULL;
		if(!(dc.font.xfont = XLoadQueryFont(dpy, fontstr)))
			eprint("error, cannot load font: '%s'\n", fontstr);
		dc.font.ascent = dc.font.xfont->ascent;
		dc.font.descent = dc.font.xfont->descent;
	}
	dc.font.height = dc.font.ascent + dc.font.descent;
}

static Bool
isoccupied(unsigned int t) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(c->tags[t])
			return True;
	return False;
}

static unsigned int
textnw(const char *text, unsigned int len) {
	XRectangle r;

	if(dc.font.set) {
		XmbTextExtents(dc.font.set, text, len, NULL, &r);
		return r.width;
	}
	return XTextWidth(dc.font.xfont, text, len);
}

static void
drawtext(const char *text, unsigned long col[ColLast]) {
	int x, y, w, h;
	static char buf[256];
	unsigned int len, olen;
	XRectangle r = { dc.x, dc.y, dc.w, dc.h };

	XSetForeground(dpy, dc.gc, col[ColBG]);
	XFillRectangles(dpy, dc.drawable, dc.gc, &r, 1);
	if(!text)
		return;
	w = 0;
	olen = len = strlen(text);
	if(len >= sizeof buf)
		len = sizeof buf - 1;
	memcpy(buf, text, len);
	buf[len] = 0;
	h = dc.font.ascent + dc.font.descent;
	y = dc.y + (dc.h / 2) - (h / 2) + dc.font.ascent;
	x = dc.x + (h / 2);
	/* shorten text if necessary */
	while(len && (w = textnw(buf, len)) > dc.w - h)
		buf[--len] = 0;
	if(len < olen) {
		if(len > 1)
			buf[len - 1] = '.';
		if(len > 2)
			buf[len - 2] = '.';
		if(len > 3)
			buf[len - 3] = '.';
	}
	if(w > dc.w)
		return; /* too long */
	XSetForeground(dpy, dc.gc, col[ColFG]);
	if(dc.font.set)
		XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, x, y, buf, len);
	else
		XDrawString(dpy, dc.drawable, dc.gc, x, y, buf, len);
}

/* extern */

unsigned int bh;
unsigned int bpos = BARPOS;
DC dc = {0};
Window barwin;

void
drawbar(void) {
	int i, x;

	dc.x = dc.y = 0;
	for(i = 0; i < ntags; i++) {
		dc.w = textw(tags[i]);
		if(seltags[i]) {
			drawtext(tags[i], dc.sel);
			drawsquare(sel && sel->tags[i], isoccupied(i), dc.sel);
		}
		else {
			drawtext(tags[i], dc.norm);
			drawsquare(sel && sel->tags[i], isoccupied(i), dc.norm);
		}
		dc.x += dc.w;
	}
	dc.w = blw;
	drawtext(getsymbol(), dc.norm);
	x = dc.x + dc.w;
	dc.w = textw(stext);
	dc.x = sw - dc.w;
	if(dc.x < x) {
		dc.x = x;
		dc.w = sw - x;
	}
	drawtext(stext, dc.norm);
	if((dc.w = dc.x - x) > bh) {
		dc.x = x;
		if(sel) {
			drawtext(sel->name, dc.sel);
			drawsquare(sel->ismax, sel->isfloating, dc.sel);
		}
		else
			drawtext(NULL, dc.norm);
	}
	XCopyArea(dpy, dc.drawable, barwin, dc.gc, 0, 0, sw, bh, 0, 0);
	XSync(dpy, False);
}

void
initbar(void) {
	XSetWindowAttributes wa;

	dc.norm[ColBorder] = initcolor(NORMBORDERCOLOR);
	dc.norm[ColBG] = initcolor(NORMBGCOLOR);
	dc.norm[ColFG] = initcolor(NORMFGCOLOR);
	dc.sel[ColBorder] = initcolor(SELBORDERCOLOR);
	dc.sel[ColBG] = initcolor(SELBGCOLOR);
	dc.sel[ColFG] = initcolor(SELFGCOLOR);
	initfont(FONT);
	dc.h = bh = dc.font.height + 2;
	wa.override_redirect = 1;
	wa.background_pixmap = ParentRelative;
	wa.event_mask = ButtonPressMask | ExposureMask;
	barwin = XCreateWindow(dpy, root, sx, sy, sw, bh, 0,
			DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
	XDefineCursor(dpy, barwin, cursor[CurNormal]);
	updatebarpos();
	XMapRaised(dpy, barwin);
	strcpy(stext, "dwm-"VERSION);
	dc.drawable = XCreatePixmap(dpy, root, sw, bh, DefaultDepth(dpy, screen));
	dc.gc = XCreateGC(dpy, root, 0, 0);
	XSetLineAttributes(dpy, dc.gc, 1, LineSolid, CapButt, JoinMiter);
	if(!dc.font.set)
		XSetFont(dpy, dc.gc, dc.font.xfont->fid);
}

unsigned int
textw(const char *text) {
	return textnw(text, strlen(text)) + dc.font.height;
}

void
togglebar(const char *arg) {
	if(bpos == BarOff)
		bpos = (BARPOS == BarOff) ? BarTop : BARPOS;
	else
		bpos = BarOff;
	updatebarpos();
	arrange();
}

void
updatebarpos(void) {
	XEvent ev;

	wax = sx;
	way = sy;
	wah = sh;
	waw = sw;
	switch(bpos) {
	default:
		wah -= bh;
		way += bh;
		XMoveWindow(dpy, barwin, sx, sy);
		break;
	case BarBot:
		wah -= bh;
		XMoveWindow(dpy, barwin, sx, sy + wah);
		break;
	case BarOff:
		XMoveWindow(dpy, barwin, sx, sy - bh);
		break;
	}
	XSync(dpy, False);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}


