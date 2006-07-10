/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * (C)opyright MMVI Sander van Dijk <a dot h dot vandijk at gmail dot com>
 * See LICENSE file for license details.
 */

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <blitz.h>
#include <cext.h>

typedef struct Item Item;

struct Item {
	Item *next;		/* traverses all items */
	Item *left, *right;	/* traverses items matching current search pattern */
	char *text;
};

static char *title = nil;
static Bool done = False;
static int ret = 0;
static char text[4096];
static BlitzColor selcolor;
static BlitzColor normcolor;
static Window win;
static XRectangle mrect;
static Item *allitem = nil;	/* first of all items */
static Item *item = nil;	/* first of pattern matching items */
static Item *sel = nil;
static Item *nextoff = nil;
static Item *prevoff = nil;
static Item *curroff = nil;
static int nitem = 0;
static unsigned int cmdw = 0;
static unsigned int twidth = 0;
static unsigned int cwidth = 0;
static Blitz blz = {0};
static BlitzBrush brush = {0};
static const int seek = 30;		/* 30px */

static void draw_menu(void);
static void handle_kpress(XKeyEvent * e);

static char version[] = "wmiimenu - " VERSION ", (C)opyright MMIV-MMVI Anselm R. Garbe\n";

static void
usage()
{
	fprintf(stderr, "%s", "usage: wmiimenu [-v] [-t <title>]\n");
	exit(1);
}

static void
update_offsets()
{
	unsigned int tw, w = cmdw + 2 * seek;

	if(!curroff)
		return;

	for(nextoff = curroff; nextoff; nextoff=nextoff->right) {
		tw = blitz_textwidth(brush.font, nextoff->text);
		if(tw > mrect.width / 3)
			tw = mrect.width / 3;
		w += tw + mrect.height;
		if(w > mrect.width)
			break;
	}

	w = cmdw + 2 * seek;
	for(prevoff = curroff; prevoff && prevoff->left; prevoff=prevoff->left) {
		tw = blitz_textwidth(brush.font, prevoff->left->text);
		if(tw > mrect.width / 3)
			tw = mrect.width / 3;
		w += tw + mrect.height;
		if(w > mrect.width)
			break;
	}
}

static void
update_items(char *pattern)
{
	unsigned int plen = strlen(pattern);
	Item *i, *j;

	if(!pattern)
		return;

	if(!title || *pattern)
		cmdw = cwidth;
	else
		cmdw = twidth;

	item = j = nil;
	nitem = 0;

	for(i = allitem; i; i=i->next)
		if(!plen || !strncmp(pattern, i->text, plen)) {
			if(!j)
				item = i;
			else
				j->right = i;
			i->left = j;
			i->right = nil;
			j = i;
			nitem++;
		}
	for(i = allitem; i; i=i->next)
		if(plen && strncmp(pattern, i->text, plen)
				&& strstr(i->text, pattern)) {
			if(!j)
				item = i;
			else
				j->right = i;
			i->left = j;
			i->right = nil;
			j = i;
			nitem++;
		}

	curroff = prevoff = nextoff = sel = item;

	update_offsets();
}

/* creates brush structs for brush mode drawing */
static void
draw_menu()
{
	unsigned int offx = 0;

	Item *i;

	brush.align = WEST;

	brush.rect = mrect;
	brush.rect.x = 0;
	brush.rect.y = 0;
	brush.color = normcolor;
	brush.border = False;
	blitz_draw_tile(&brush);

	/* print command */
	if(!title || text[0]) {
		brush.color = normcolor;
		cmdw = cwidth;
		if(cmdw && item)
			brush.rect.width = cmdw;
		blitz_draw_label(&brush, text);
	}
	else {
		cmdw = twidth;
		brush.color = selcolor;
		brush.rect.width = cmdw;
		blitz_draw_label(&brush, title);
	}
	offx += brush.rect.width;

	brush.align = CENTER;
	if(curroff) {
		brush.color = normcolor;
		brush.rect.x = offx;
		brush.rect.width = seek;
		offx += brush.rect.width;
		blitz_draw_label(&brush, (curroff && curroff->left) ? "<" : nil);

		/* determine maximum items */
		for(i = curroff; i != nextoff; i=i->right) {
			brush.color = normcolor;
			brush.border = False;
			brush.rect.x = offx;
			brush.rect.width = blitz_textwidth(brush.font, i->text);
			if(brush.rect.width > mrect.width / 3)
				brush.rect.width = mrect.width / 3;
			brush.rect.width += mrect.height;
			if(sel == i) {
				brush.color = selcolor;
				brush.border = True;
			}
			blitz_draw_label(&brush, i->text);
			offx += brush.rect.width;
		}

		brush.color = normcolor;
		brush.border = False;
		brush.rect.x = mrect.width - seek;
		brush.rect.width = seek;
		blitz_draw_label(&brush, nextoff ? ">" : nil);
	}
	XCopyArea(blz.dpy, brush.drawable, win, brush.gc, 0, 0, mrect.width,
			mrect.height, 0, 0);
	XSync(blz.dpy, False);
}

static void
handle_kpress(XKeyEvent * e)
{
	KeySym ksym;
	char buf[32];
	int num, prev_nitem;
	unsigned int i, len = strlen(text);

	buf[0] = 0;
	num = XLookupString(e, buf, sizeof(buf), &ksym, 0);

	if(IsFunctionKey(ksym) || IsKeypadKey(ksym)
			|| IsMiscFunctionKey(ksym) || IsPFKey(ksym)
			|| IsPrivateKeypadKey(ksym))
		return;

	/* first check if a control mask is omitted */
	if(e->state & ControlMask) {
		switch (ksym) {
		case XK_H:
		case XK_h:
			ksym = XK_BackSpace;
			break;
		case XK_I:
		case XK_i:
			ksym = XK_Tab;
			break;
		case XK_J:
		case XK_j:
			ksym = XK_Return;
			break;
		case XK_N:
		case XK_n:
			ksym = XK_Right;
			break;
		case XK_P:
		case XK_p:
			ksym = XK_Left;
			break;
		case XK_U:
		case XK_u:
			text[0] = 0;
			update_items(text);
			draw_menu();
			return;
			break;
		case XK_bracketleft:
			ksym = XK_Escape;
			break;
		default:	/* ignore other control sequences */
			return;
			break;
		}
	}
	switch (ksym) {
	case XK_Left:
		if(!(sel && sel->left))
			return;
		sel=sel->left;
		if(sel->right == curroff) {
			curroff = prevoff;
			update_offsets();
		}
		break;
	case XK_Tab:
		if(!sel)
			return;
		cext_strlcpy(text, sel->text, sizeof(text));
		update_items(text);
		break;
	case XK_Right:
		if(!(sel && sel->right))
			return;
		sel=sel->right;
		if(sel == nextoff) {
			curroff = nextoff;
			update_offsets();
		}
		break;
	case XK_Return:
		if(e->state & ShiftMask) {
			if(text)
				fprintf(stdout, "%s", text);
		}
		else if(sel)
			fprintf(stdout, "%s", sel->text);
		else if(text)
			fprintf(stdout, "%s", text);
		fflush(stdout);
		done = True;
		break;
	case XK_Escape:
		ret = 1;
		done = True;
		break;
	case XK_BackSpace:
		if((i = len)) {
			prev_nitem = nitem;
			do {
				text[--i] = 0;
				update_items(text);
			} while(i && nitem && prev_nitem == nitem);
			update_items(text);
		}
		break;
	default:
		if((num == 1) && !iscntrl((int) buf[0])) {
			buf[num] = 0;
			if(len > 0)
				cext_strlcat(text, buf, sizeof(text));
			else
				cext_strlcpy(text, buf, sizeof(text));
			update_items(text);
		}
	}
	draw_menu();
}

static char *
read_allitems()
{
	static char *maxname = nil;
	char *p, buf[1024];
	unsigned int len = 0, max = 0;
	Item *i, *new;

	i = nil;
	while(fgets(buf, sizeof(buf), stdin)) {
		len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = 0;
		p = cext_estrdup(buf);
		if(max < len) {
			maxname = p;
			max = len;
		}

		new = cext_emalloc(sizeof(Item));
		new->next = new->left = new->right = nil;
		new->text = p;
		if(!i)
			allitem = new;
		else 
			i->next = new;
		i = new;
	}

	return maxname;
}

int
main(int argc, char *argv[])
{
	int i;
	XSetWindowAttributes wa;
	char *maxname, *p;
	BlitzFont font = {0};
	GC gc;
	Drawable pmap;
	XEvent ev;

	/* command line args */
	for(i = 1; i < argc; i++) {
		if (argv[i][0] == '-')
			switch (argv[i][1]) {
			case 'v':
				fprintf(stdout, "%s", version);
				exit(0);
				break;
			case 't':
				if(++i < argc)
					title = argv[i];
				else
					usage();
				break;
			default:
				usage();
				break;
			}
		else
			usage();
	}

	blz.dpy = XOpenDisplay(0);
	if(!blz.dpy) {
		fprintf(stderr, "%s", "wmiimenu: cannot open dpy\n");
		exit(1);
	}
	blz.screen = DefaultScreen(blz.dpy);
	blz.root = RootWindow(blz.dpy, blz.screen);

	maxname = read_allitems();

	/* grab as early as possible, but after reading all items!!! */
	while(XGrabKeyboard
			(blz.dpy, blz.root, True, GrabModeAsync,
			 GrabModeAsync, CurrentTime) != GrabSuccess)
		usleep(1000);

	font.fontstr = getenv("WMII_FONT");
	if (!font.fontstr)
		font.fontstr = cext_estrdup(BLITZ_FONT);
	blitz_loadfont(&blz, &font);

	if((p = getenv("WMII_NORMCOLORS")))
		cext_strlcpy(normcolor.colstr, p, sizeof(normcolor.colstr));
	if(strlen(normcolor.colstr) != 23)
		cext_strlcpy(normcolor.colstr, BLITZ_NORMCOLORS, sizeof(normcolor.colstr));
	blitz_loadcolor(&blz, &normcolor);

	if((p = getenv("WMII_SELCOLORS")))
		cext_strlcpy(selcolor.colstr, p, sizeof(selcolor.colstr));
	if(strlen(selcolor.colstr) != 23)
		cext_strlcpy(selcolor.colstr, BLITZ_SELCOLORS, sizeof(selcolor.colstr));
	blitz_loadcolor(&blz, &selcolor);

	wa.override_redirect = 1;
	wa.background_pixmap = ParentRelative;
	wa.event_mask = ExposureMask | ButtonPressMask | KeyPressMask
		| SubstructureRedirectMask | SubstructureNotifyMask;

	mrect.width = DisplayWidth(blz.dpy, blz.screen);
	mrect.height = font.ascent + font.descent + 4;
	mrect.y = DisplayHeight(blz.dpy, blz.screen) - mrect.height;
	mrect.x = 0;

	win = XCreateWindow(blz.dpy, blz.root, mrect.x, mrect.y,
			mrect.width, mrect.height, 0, DefaultDepth(blz.dpy, blz.screen),
			CopyFromParent, DefaultVisual(blz.dpy, blz.screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
	XDefineCursor(blz.dpy, win, XCreateFontCursor(blz.dpy, XC_xterm));
	XSync(blz.dpy, False);

	/* pixmap */
	gc = XCreateGC(blz.dpy, win, 0, 0);
	pmap = XCreatePixmap(blz.dpy, win, mrect.width, mrect.height,
			DefaultDepth(blz.dpy, blz.screen));

	XSync(blz.dpy, False);

	brush.blitz = &blz;
	brush.color = normcolor;
	brush.drawable = pmap;
	brush.gc = gc;
	brush.font = &font;

	if(maxname)
		cwidth = blitz_textwidth(brush.font, maxname) + mrect.height;
	if(cwidth > mrect.width / 3)
		cwidth = mrect.width / 3;

	if(title) {
		twidth = blitz_textwidth(brush.font, title) + mrect.height;
		if(twidth > mrect.width / 3)
			twidth = mrect.width / 3;
	}

	cmdw = title ? twidth : cwidth;

	text[0] = 0;
	update_items(text);
	XMapRaised(blz.dpy, win);
	draw_menu();
	XSync(blz.dpy, False);

	/* main event loop */
	while(!XNextEvent(blz.dpy, &ev)) {
		switch (ev.type) {
			case KeyPress:
				handle_kpress(&ev.xkey);
				break;
			case Expose:
				if(ev.xexpose.count == 0) {
					draw_menu();
				}
				break;
			default:
				break;
		}
		if(done)
			break;
	}

	XUngrabKeyboard(blz.dpy, CurrentTime);
	XFreePixmap(blz.dpy, pmap);
	XFreeGC(blz.dpy, gc);
	XDestroyWindow(blz.dpy, win);
	XCloseDisplay(blz.dpy);

	return ret;
}
