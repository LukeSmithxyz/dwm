/* See LICENSE file for copyright and license details. */
int bx, by, bw, bh, blw, mx, my, mw, mh, tx, ty, tw, th, wx, wy, ww, wh;

void setmfact(const char *arg);
void tile(void);
void tileresize(Client *c, int x, int y, int w, int h);
void updatetilegeom(void);

void
setmfact(const char *arg) {
	double d;

	if(!arg || lt->arrange != tile)
		return;
	else {
		d = strtod(arg, NULL);
		if(arg[0] == '-' || arg[0] == '+')
			d += mfact;
		if(d < 0.1 || d > 0.9)
			return;
		mfact = d;
	}
	updategeom();
	arrange();
}

void
tile(void) {
	int y, h;
	unsigned int i, n;
	Client *c;

	for(n = 0, c = nextunfloating(clients); c; c = nextunfloating(c->next), n++);
	if(n == 0)
		return;

	/* master */
	c = nextunfloating(clients);

	if(n == 1)
		tileresize(c, wx, wy, ww - 2 * c->bw, wh - 2 * c->bw);
	else
		tileresize(c, mx, my, mw - 2 * c->bw, mh - 2 * c->bw);

	if(--n == 0)
		return;

	/* tile stack */
	y = ty;
	h = th / n;
	if(h < bh)
		h = th;

	for(i = 0, c = nextunfloating(c->next); c; c = nextunfloating(c->next), i++) {
		if(i + 1 == n) /* remainder */
			tileresize(c, tx, y, tw - 2 * c->bw, (ty + th) - y - 2 * c->bw);
		else
			tileresize(c, tx, y, tw - 2 * c->bw, h - 2 * c->bw);
		if(h != th)
			y = c->y + c->h + 2 * c->bw;
	}
}

void
tileresize(Client *c, int x, int y, int w, int h) {
	resize(c, x, y, w, h, resizehints);
	if(resizehints && ((c->h < bh) || (c->h > h) || (c->w < bh) || (c->w > w)))
		/* client doesn't accept size constraints */
		resize(c, x, y, w, h, False);
}

void
zoom(const char *arg) {
	Client *c = sel;

	if(c == nextunfloating(clients))
		if(!c || !(c = nextunfloating(c->next)))
			return;
	if(lt->arrange == tile && !sel->isfloating) {
		detach(c);
		attach(c);
		focus(c);
	}
	arrange();
}

void
updatetilegeom(void) {
#ifdef TILEGEOM /* define your own if you are Xinerama user */
	TILEGEOM
#else
	/* master area geometry */
	mx = wx;
	my = wy;
	mw = mfact * ww;
	mh = wh;

	/* tile area geometry */
	tx = mx + mw;
	ty = wy;
	tw = ww - mw;
	th = wh;
#endif
}
