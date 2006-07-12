/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "wm.h"

#include <X11/keysym.h>

static const char *term[] = { 
	"xterm", "-bg", "black", "-fg", "white", "-fn",
	"-*-terminus-medium-*-*-*-13-*-*-*-*-*-iso10646-*", 0 
};

static const char *proglist[] = {
		"sh", "-c", "exec `ls -lL /bin /sbin /usr/bin /usr/local/bin 2>/dev/null "
		"| awk 'NF>2 && $1 ~ /^[^d].*x/ {print $NF}' | sort | uniq | gridmenu`", 0
};

static Key key[] = {
	{ Mod1Mask, XK_Return, run, term },
	{ Mod1Mask, XK_p, run, proglist }, 
	{ Mod1Mask, XK_k, sel, "prev" }, 
	{ Mod1Mask, XK_j, sel, "next" }, 
	{ Mod1Mask, XK_g, arrange, NULL }, 
	{ Mod1Mask | ShiftMask, XK_c, kill, NULL }, 
	{ Mod1Mask | ShiftMask, XK_q, quit, NULL },
};

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
