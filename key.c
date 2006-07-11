/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include "wm.h"

#include <X11/keysym.h>

static Key key[] = {
	KEYS
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
				key[i].func(key[i].arg);
			return;
		}
}
