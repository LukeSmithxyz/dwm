/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "wm.h"

#include <X11/keysym.h>

/********** CUSTOMIZE **********/

char *term[] = { 
	"aterm", "-tr", "+sb", "-bg", "black", "-fg", "white", "-fn",
	"-*-terminus-medium-*-*-*-13-*-*-*-*-*-iso10646-*",NULL
};

static Key key[] = {
	{ Mod1Mask, XK_Return, run, term },
	{ Mod1Mask, XK_k, sel, "prev" }, 
	{ Mod1Mask, XK_j, sel, "next" }, 
	{ Mod1Mask, XK_g, grid, NULL }, 
	{ Mod1Mask, XK_f, floating, NULL }, 
	{ Mod1Mask, XK_m, max, NULL }, 
	{ Mod1Mask | ShiftMask, XK_c, ckill, NULL }, 
	{ Mod1Mask | ShiftMask, XK_q, quit, NULL },
};

/********** CUSTOMIZE **********/

void
update_keys()
{
	unsigned int i, len;
	KeyCode code;

	len = sizeof(key) / sizeof(key[0]);
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
	unsigned int i, len;
	KeySym keysym;

	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
	len = sizeof(key) / sizeof(key[0]);
	for(i = 0; i < len; i++)
		if((keysym == key[i].keysym) && (key[i].mod == ev->state)) {
			if(key[i].func)
				key[i].func(key[i].aux);
			return;
		}
}
