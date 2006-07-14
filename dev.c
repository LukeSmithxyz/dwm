/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "dwm.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/keysym.h>

/********** CUSTOMIZE **********/

const char *term[] = { 
	"urxvtc", "-tr", "+sb", "-bg", "black", "-fg", "white", "-fn",
	"-*-terminus-medium-*-*-*-13-*-*-*-*-*-iso10646-*",NULL
};
const char *browse[] = { "firefox", NULL };
const char *xlock[] = { "xlock", NULL };

static Key key[] = {
	/* modifier				key			function	arguments */
	{ Mod1Mask,				XK_Return,	zoom,		{ 0 } },
	{ Mod1Mask,				XK_k,		prevc,		{ 0 } },
	{ Mod1Mask,				XK_j,		nextc,		{ 0 } }, 
	{ Mod1Mask,				XK_m,		max,		{ 0 } }, 
	{ Mod1Mask,				XK_0,		view,		{ .i = Tscratch } }, 
	{ Mod1Mask,				XK_1,		view,		{ .i = Tdev } }, 
	{ Mod1Mask,				XK_2,		view,		{ .i = Twww } }, 
	{ Mod1Mask,				XK_3,		view,		{ .i = Twork } }, 
	{ Mod1Mask,				XK_space,	tiling,		{ 0 } }, 
	{ Mod1Mask|ShiftMask,	XK_space,	floating,	{ 0 } }, 
	{ Mod1Mask|ShiftMask,	XK_0,		ttrunc,		{ .i = Tscratch } }, 
	{ Mod1Mask|ShiftMask,	XK_1,		ttrunc,		{ .i = Tdev } }, 
	{ Mod1Mask|ShiftMask,	XK_2,		ttrunc,		{ .i = Twww } }, 
	{ Mod1Mask|ShiftMask,	XK_3,		ttrunc,		{ .i = Twork } }, 
	{ Mod1Mask|ShiftMask,	XK_c,		ckill,		{ 0 } }, 
	{ Mod1Mask|ShiftMask,	XK_q,		quit,		{ 0 } },
	{ Mod1Mask|ShiftMask,	XK_Return,	spawn,		{ .argv = term } },
	{ Mod1Mask|ShiftMask,	XK_w,		spawn,		{ .argv = browse } },
	{ Mod1Mask|ShiftMask,	XK_l,		spawn,		{ .argv = xlock } },
	{ ControlMask,			XK_0,		tappend,	{ .i = Tscratch } }, 
	{ ControlMask,			XK_1,		tappend,	{ .i = Tdev } }, 
	{ ControlMask,			XK_2,		tappend,	{ .i = Twww } }, 
	{ ControlMask,			XK_3,		tappend,	{ .i = Twork } }, 
};

/********** CUSTOMIZE **********/

void
update_keys(void)
{
	static unsigned int len = key ? sizeof(key) / sizeof(key[0]) : 0;
	unsigned int i;
	KeyCode code;

	for(i = 0; i < len; i++) {
		code = XKeysymToKeycode(dpy, key[i].keysym);
		XUngrabKey(dpy, code, key[i].mod, root);
		XGrabKey(dpy, code, key[i].mod, root, True, GrabModeAsync, GrabModeAsync);
	}
}

void
keypress(XEvent *e)
{
	XKeyEvent *ev = &e->xkey;
	static unsigned int len = key ? sizeof(key) / sizeof(key[0]) : 0;
	unsigned int i;
	KeySym keysym;

	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
	for(i = 0; i < len; i++)
		if((keysym == key[i].keysym) && (key[i].mod == ev->state)) {
			if(key[i].func)
				key[i].func(&key[i].arg);
			return;
		}
}

#define ButtonMask      (ButtonPressMask | ButtonReleaseMask)
#define MouseMask       (ButtonMask | PointerMotionMask)

void
mresize(Client *c)
{
	XEvent ev;
	int ocx, ocy;

	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MouseMask, GrabModeAsync, GrabModeAsync,
				None, cursor[CurResize], CurrentTime) != GrabSuccess)
		return;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w, c->h);
	for(;;) {
		XMaskEvent(dpy, MouseMask | ExposureMask, &ev);
		switch(ev.type) {
		default: break;
		case Expose:
			handler[Expose](&ev);
			break;
		case MotionNotify:
			XFlush(dpy);
			c->w = abs(ocx - ev.xmotion.x);
			c->h = abs(ocy - ev.xmotion.y);
			c->x = (ocx <= ev.xmotion.x) ? ocx : ocx - c->w;
			c->y = (ocy <= ev.xmotion.y) ? ocy : ocy - c->h;
			resize(c, True);
			break;
		case ButtonRelease:
			XUngrabPointer(dpy, CurrentTime);
			return;
		}
	}
}

void
mmove(Client *c)
{
	XEvent ev;
	int x1, y1, ocx, ocy, di;
	unsigned int dui;
	Window dummy;

	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MouseMask, GrabModeAsync, GrabModeAsync,
				None, cursor[CurMove], CurrentTime) != GrabSuccess)
		return;
	XQueryPointer(dpy, root, &dummy, &dummy, &x1, &y1, &di, &di, &dui);
	for(;;) {
		XMaskEvent(dpy, MouseMask | ExposureMask, &ev);
		switch (ev.type) {
		default: break;
		case Expose:
			handler[Expose](&ev);
			break;
		case MotionNotify:
			XFlush(dpy);
			c->x = ocx + (ev.xmotion.x - x1);
			c->y = ocy + (ev.xmotion.y - y1);
			resize(c, False);
			break;
		case ButtonRelease:
			XUngrabPointer(dpy, CurrentTime);
			return;
		}
	}
}
