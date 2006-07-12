/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * (C)opyright MMVI Sander van Dijk <a dot h dot vandijk at gmail dot com>
 * See LICENSE file for license details.
 */

#include "config.h"
#include "draw.h"
#include "util.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

typedef struct Item Item;

struct Item {
	Item *next;		/* traverses all items */
	Item *left, *right;	/* traverses items matching current search pattern */
	char *text;
};

static Display *dpy;
static Window root;
static Window win;
static Bool done = False;

static Item *allitem = NULL;	/* first of all items */
static Item *item = NULL;	/* first of pattern matching items */
static Item *sel = NULL;
static Item *nextoff = NULL;
static Item *prevoff = NULL;
static Item *curroff = NULL;

static int screen, mx, my, mw, mh;
static char *title = NULL;
static char text[4096];
static int ret = 0;
static int nitem = 0;
static unsigned int cmdw = 0;
static unsigned int tw = 0;
static unsigned int cw = 0;
static const int seek = 30;		/* 30px */

static Brush brush = {0};

static void draw_menu();
static void kpress(XKeyEvent * e);

static char version[] = "gridmenu - " VERSION ", (C)opyright MMVI Anselm R. Garbe\n";

static void
usage()
{
	fprintf(stderr, "%s", "usage: gridmenu [-v] [-t <title>]\n");
	exit(1);
}

static void
update_offsets()
{
	unsigned int tw, w = cmdw + 2 * seek;

	if(!curroff)
		return;

	for(nextoff = curroff; nextoff; nextoff=nextoff->right) {
		tw = textw(&brush.font, nextoff->text);
		if(tw > mw / 3)
			tw = mw / 3;
		w += tw + brush.font.height;
		if(w > mw)
			break;
	}

	w = cmdw + 2 * seek;
	for(prevoff = curroff; prevoff && prevoff->left; prevoff=prevoff->left) {
		tw = textw(&brush.font, prevoff->left->text);
		if(tw > mw / 3)
			tw = mw / 3;
		w += tw + brush.font.height;
		if(w > mw)
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
		cmdw = cw;
	else
		cmdw = tw;

	item = j = NULL;
	nitem = 0;

	for(i = allitem; i; i=i->next)
		if(!plen || !strncmp(pattern, i->text, plen)) {
			if(!j)
				item = i;
			else
				j->right = i;
			i->left = j;
			i->right = NULL;
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
			i->right = NULL;
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
	Item *i;

	brush.x = 0;
	brush.y = 0;
	brush.w = mw;
	brush.h = mh;
	draw(dpy, &brush, False, 0);

	/* print command */
	if(!title || text[0]) {
		cmdw = cw;
		if(cmdw && item)
			brush.w = cmdw;
		draw(dpy, &brush, False, text);
	}
	else {
		cmdw = tw;
		brush.w = cmdw;
		draw(dpy, &brush, False, title);
	}
	brush.x += brush.w;

	if(curroff) {
		brush.w = seek;
		draw(dpy, &brush, False, (curroff && curroff->left) ? "<" : 0);
		brush.x += brush.w;

		/* determine maximum items */
		for(i = curroff; i != nextoff; i=i->right) {
			brush.border = False;
			brush.w = textw(&brush.font, i->text);
			if(brush.w > mw / 3)
				brush.w = mw / 3;
			brush.w += brush.font.height;
			if(sel == i) {
				swap((void **)&brush.fg, (void **)&brush.bg);
				draw(dpy, &brush, True, i->text);
				swap((void **)&brush.fg, (void **)&brush.bg);
			}
			else
				draw(dpy, &brush, False, i->text);
			brush.x += brush.w;
		}

		brush.x = mw - seek;
		brush.w = seek;
		draw(dpy, &brush, False, nextoff ? ">" : 0);
	}
	XCopyArea(dpy, brush.drawable, win, brush.gc, 0, 0, mw, mh, 0, 0);
	XFlush(dpy);
}

static void
kpress(XKeyEvent * e)
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
		strncpy(text, sel->text, sizeof(text));
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
		if(num && !iscntrl((int) buf[0])) {
			buf[num] = 0;
			if(len > 0)
				strncat(text, buf, sizeof(text));
			else
				strncpy(text, buf, sizeof(text));
			update_items(text);
		}
	}
	draw_menu();
}

static char *
read_allitems()
{
	static char *maxname = NULL;
	char *p, buf[1024];
	unsigned int len = 0, max = 0;
	Item *i, *new;

	i = 0;
	while(fgets(buf, sizeof(buf), stdin)) {
		len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = 0;
		p = estrdup(buf);
		if(max < len) {
			maxname = p;
			max = len;
		}

		new = emalloc(sizeof(Item));
		new->next = new->left = new->right = NULL;
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
	char *maxname;
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

	dpy = XOpenDisplay(0);
	if(!dpy)
		error("gridmenu: cannot open dpy\n");
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);

	maxname = read_allitems();

	/* grab as early as possible, but after reading all items!!! */
	while(XGrabKeyboard(dpy, root, True, GrabModeAsync,
			 GrabModeAsync, CurrentTime) != GrabSuccess)
		usleep(1000);

	/* style */
	loadcolors(dpy, screen, &brush, BGCOLOR, FGCOLOR, BORDERCOLOR);
	loadfont(dpy, &brush.font, FONT);

	wa.override_redirect = 1;
	wa.background_pixmap = ParentRelative;
	wa.event_mask = ExposureMask | ButtonPressMask | KeyPressMask;

	mx = my = 0;
	mw = DisplayWidth(dpy, screen);
	mh = texth(&brush.font);

	win = XCreateWindow(dpy, root, mx, my, mw, mh, 0,
			DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
	XDefineCursor(dpy, win, XCreateFontCursor(dpy, XC_xterm));
	XFlush(dpy);

	/* pixmap */
	brush.gc = XCreateGC(dpy, root, 0, 0);
	brush.drawable = XCreatePixmap(dpy, win, mw, mh,
			DefaultDepth(dpy, screen));
	XFlush(dpy);

	if(maxname)
		cw = textw(&brush.font, maxname) + brush.font.height;
	if(cw > mw / 3)
		cw = mw / 3;

	if(title) {
		tw = textw(&brush.font, title) + brush.font.height;
		if(tw > mw / 3)
			tw = mw / 3;
	}

	cmdw = title ? tw : cw;

	text[0] = 0;
	update_items(text);
	XMapRaised(dpy, win);
	draw_menu();
	XFlush(dpy);

	/* main event loop */
	while(!XNextEvent(dpy, &ev)) {
		switch (ev.type) {
			case KeyPress:
				kpress(&ev.xkey);
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

	XUngrabKeyboard(dpy, CurrentTime);
	XFreePixmap(dpy, brush.drawable);
	XFreeGC(dpy, brush.gc);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

	return ret;
}
