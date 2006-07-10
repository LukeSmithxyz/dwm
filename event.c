/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <X11/keysym.h>

#include "wm.h"

/* local functions */
static void configurerequest(XEvent *e);
static void destroynotify(XEvent *e);
static void enternotify(XEvent *e);
static void leavenotify(XEvent *e);
static void expose(XEvent *e);
static void keypress(XEvent *e);
static void keymapnotify(XEvent *e);
static void maprequest(XEvent *e);
static void propertynotify(XEvent *e);
static void unmapnotify(XEvent *e);

void (*handler[LASTEvent]) (XEvent *) = {
	[ConfigureRequest] = configurerequest,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = enternotify,
	[LeaveNotify] = leavenotify,
	[Expose] = expose,
	[KeyPress] = keypress,
	[KeymapNotify] = keymapnotify,
	[MapRequest] = maprequest,
	[PropertyNotify] = propertynotify,
	[UnmapNotify] = unmapnotify
};

unsigned int
flush_masked_events(long even_mask)
{
	XEvent ev;
	unsigned int n = 0;
	while(XCheckMaskEvent(dpy, even_mask, &ev)) n++;
	return n;
}

static void
configurerequest(XEvent *e)
{
#if 0
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;
	XRectangle *frect;
	Client *c;

	c = client_of_win(ev->window);
	ev->value_mask &= ~CWSibling;
	if(c) {
		gravitate_client(c, True);

		if(ev->value_mask & CWX)
			c->rect.x = ev->x;
		if(ev->value_mask & CWY)
			c->rect.y = ev->y;
		if(ev->value_mask & CWWidth)
			c->rect.width = ev->width;
		if(ev->value_mask & CWHeight)
			c->rect.height = ev->height;
		if(ev->value_mask & CWBorderWidth)
			c->border = ev->border_width;

		gravitate_client(c, False);

		if(c->frame) {
			if(c->sel->area->floating)
				frect=&c->sel->rect;
			else
				frect=&c->sel->revert;

			if(c->rect.width >= screen->rect.width && c->rect.height >= screen->rect.height) {
				frect->y = wc.y = -height_of_bar();
				frect->x = wc.x = -def.border;
			}
			else {
				frect->y = wc.y = c->rect.y - height_of_bar();
				frect->x = wc.x = c->rect.x - def.border;
			}
			frect->width = wc.width = c->rect.width + 2 * def.border;
			frect->height = wc.height = c->rect.height + def.border
				+ height_of_bar();
			wc.border_width = 1;
			wc.sibling = None;
			wc.stack_mode = ev->detail;
			if(c->sel->area->view != screen->sel)
				wc.x += 2 * screen->rect.width;
			if(c->sel->area->floating) {
				XConfigureWindow(dpy, c->framewin, ev->value_mask, &wc);
				configure_client(c);
			}
		}
	}

	wc.x = ev->x;
	wc.y = ev->y;
	wc.width = ev->width;
	wc.height = ev->height;

	if(c && c->frame) {
		wc.x = def.border;
		wc.y = height_of_bar();
		wc.width = c->sel->rect.width - 2 * def.border;
		wc.height = c->sel->rect.height - def.border - height_of_bar();
	}

	wc.border_width = 0;
	wc.sibling = None;
	wc.stack_mode = Above;
	ev->value_mask &= ~CWStackMode;
	ev->value_mask |= CWBorderWidth;
	XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);

	XFlush(dpy);
#endif
}

static void
destroynotify(XEvent *e)
{
#if 0
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if((c = client_of_win(ev->window)))
		destroy_client(c);
#endif
}

static void
enternotify(XEvent *e)
{
#if 0
	XCrossingEvent *ev = &e->xcrossing;
	Client *c;

	if(ev->mode != NotifyNormal || ev->detail == NotifyInferior)
		return;

	if((c = client_of_win(ev->window))) {
		Frame *f = c->sel;
		Area *a = f->area;
		if(a->mode == Colmax)
			c = a->sel->client;
		focus(c, False);
	}
	else if(ev->window == root) {
		sel_screen = True;
		draw_frames();
	}
#endif
}

static void
leavenotify(XEvent *e)
{
	XCrossingEvent *ev = &e->xcrossing;

	if((ev->window == root) && !ev->same_screen) {
		sel_screen = True;
		/*draw_frames();*/
	}
}

static void
expose(XEvent *e)
{
	XExposeEvent *ev = &e->xexpose;

	if(ev->count == 0) {
		if(ev->window == barwin)
			draw_bar();
	}
}

static void
keypress(XEvent *e)
{
#if 0
	XKeyEvent *ev = &e->xkey;
	KeySym k = 0;
	char buf[32];
	int n;
	static Frame *f;


	ev->state &= valid_mask;
	if((f = frame_of_win(ev->window))) {
		buf[0] = 0;
		n = XLookupString(ev, buf, sizeof(buf), &k, 0);
		if(IsFunctionKey(k) || IsKeypadKey(k) || IsMiscFunctionKey(k)
				|| IsPFKey(k) || IsPrivateKeypadKey(k))
			return;
		buf[n] = 0;
		blitz_kpress_input(&f->tagbar, ev->state, k, buf);
	}
	else
		key(root, ev->state, (KeyCode) ev->keycode);
#endif
}

static void
keymapnotify(XEvent *e)
{
#if 0
	update_keys();
#endif
}

static void
maprequest(XEvent *e)
{
#if 0
	XMapRequestEvent *ev = &e->xmaprequest;
	static XWindowAttributes wa;

	if(!XGetWindowAttributes(dpy, ev->window, &wa))
		return;

	if(wa.override_redirect) {
		XSelectInput(dpy, ev->window,
				(StructureNotifyMask | PropertyChangeMask));
		return;
	}

	if(!client_of_win(ev->window))
		manage_client(create_client(ev->window, &wa));
#endif
}

static void
propertynotify(XEvent *e)
{
#if 0
	XPropertyEvent *ev = &e->xproperty;
	Client *c;

	if(ev->state == PropertyDelete)
		return; /* ignore */

	if((c = client_of_win(ev->window)))
		prop_client(c, ev);
#endif
}

static void
unmapnotify(XEvent *e)
{
#if 0
	Client *c;
	XUnmapEvent *ev = &e->xunmap;

	if((c = client_of_win(ev->window)))
		destroy_client(c);
#endif
}
