/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#include "dwm.h"

static void ckill(Arg *arg);
static void nextc(Arg *arg);
static void prevc(Arg *arg);
static void max(Arg *arg);
static void ttrunc(Arg *arg);
static void tappend(Arg *arg);
static void zoom(Arg *arg);

/********** CUSTOMIZE **********/

const char *term[] = { 
	"urxvtc", "-tr", "+sb", "-bg", "black", "-fg", "white", "-fn",
	"-*-terminus-medium-*-*-*-13-*-*-*-*-*-iso10646-*",NULL
};
const char *browse[] = { "firefox", NULL };
const char *xlock[] = { "xlock", NULL };

Key key[] = {
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
grabkeys()
{
	static unsigned int len = key ? sizeof(key) / sizeof(key[0]) : 0;
	unsigned int i;
	KeyCode code;

	for(i = 0; i < len; i++) {
		code = XKeysymToKeycode(dpy, key[i].keysym);
		XUngrabKey(dpy, code, key[i].mod, root);
		XGrabKey(dpy, code, key[i].mod, root, True,
				GrabModeAsync, GrabModeAsync);
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

static void
zoom(Arg *arg)
{
	Client **l, *c;

	if(!sel)
		return;

	if(sel == getnext(clients) && sel->next)  {
		if((c = getnext(sel->next)))
			sel = c;
	}

	for(l = &clients; *l && *l != sel; l = &(*l)->next);
	*l = sel->next;

	sel->next = clients; /* pop */
	clients = sel;
	arrange(NULL);
	focus(sel);
}

static void
max(Arg *arg)
{
	if(!sel)
		return;
	sel->x = sx;
	sel->y = sy + bh;
	sel->w = sw - 2 * sel->border;
	sel->h = sh - 2 * sel->border - bh;
	higher(sel);
	resize(sel, False);
}

static void
tappend(Arg *arg)
{
	if(!sel)
		return;

	sel->tags[arg->i] = tags[arg->i];
	arrange(NULL);
}

static void
ttrunc(Arg *arg)
{
	int i;
	if(!sel)
		return;

	for(i = 0; i < TLast; i++)
		sel->tags[i] = NULL;
	tappend(arg);
}

static void
prevc(Arg *arg)
{
	Client *c;

	if(!sel)
		return;

	if((c = sel->revert && sel->revert->tags[tsel] ? sel->revert : NULL)) {
		higher(c);
		focus(c);
	}
}

static void
nextc(Arg *arg)
{
	Client *c;
   
	if(!sel)
		return;

	if(!(c = getnext(sel->next)))
		c = getnext(clients);
	if(c) {
		higher(c);
		c->revert = sel;
		focus(c);
	}
}

static void
ckill(Arg *arg)
{
	if(!sel)
		return;
	if(sel->proto & WM_PROTOCOL_DELWIN)
		sendevent(sel->win, wm_atom[WMProtocols], wm_atom[WMDelete]);
	else
		XKillClient(dpy, sel->win);
}

