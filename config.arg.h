/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#define FONT			"-*-terminus-medium-*-*-*-13-*-*-*-*-*-iso10646-*"
#define BGCOLOR			"#0a2c2d"
#define FGCOLOR			"#ddeeee"
#define BORDERCOLOR		"#176164"
#define MODKEY			Mod1Mask
#define NUMLOCKMASK		Mod2Mask
#define MASTERW			52 /* percent */
#define WM_PROTOCOL_DELWIN	1

enum { Tfnord, Tdev, Tnet, Twork, Tmisc, TLast };
#define TAGS \
char *tags[TLast] = { \
	[Tfnord] = "fnord", \
	[Tdev] = "dev", \
	[Tnet] = "net", \
	[Twork] = "work", \
	[Tmisc] = "misc", \
};
#define DEFTAG Tdev


#define CMDS \
	const char *browse[] = { "firefox", NULL }; \
	const char *gimp[] = { "gimp", NULL }; \
	const char *term[] = { \
		"urxvt", "-tr", "+sb", "-bg", "black", "-fg", "white", "-cr", "white", \
		"-fn", "-*-terminus-medium-*-*-*-13-*-*-*-*-*-iso10646-*", NULL \
	}; \
	const char *xlock[] = { "xlock", NULL };

#define KEYS \
static Key key[] = { \
	/* modifier		key		function	arguments */ \
	{ MODKEY,		XK_0,		view,		{ .i = Tfnord } }, \
	{ MODKEY,		XK_1,		view,		{ .i = Tdev } }, \
	{ MODKEY,		XK_2,		view,		{ .i = Tnet } }, \
	{ MODKEY,		XK_3,		view,		{ .i = Twork } }, \
	{ MODKEY,		XK_4,		view,		{ .i = Tmisc} }, \
	{ MODKEY,		XK_h,		viewprev,	{ 0 } }, \
	{ MODKEY,		XK_j,		focusnext,	{ 0 } }, \
	{ MODKEY,		XK_k,		focusprev,	{ 0 } }, \
	{ MODKEY,		XK_l,		viewnext,	{ 0 } }, \
	{ MODKEY,		XK_m,		togglemax,	{ 0 } }, \
	{ MODKEY,		XK_space,	togglemode,	{ 0 } }, \
	{ MODKEY,		XK_Return,	zoom,		{ 0 } }, \
	{ MODKEY|ControlMask,	XK_0,		appendtag,	{ .i = Tfnord } }, \
	{ MODKEY|ControlMask,	XK_1,		appendtag,	{ .i = Tdev } }, \
	{ MODKEY|ControlMask,	XK_2,		appendtag,	{ .i = Tnet } }, \
	{ MODKEY|ControlMask,	XK_3,		appendtag,	{ .i = Twork } }, \
	{ MODKEY|ControlMask,	XK_4,		appendtag,	{ .i = Tmisc } }, \
	{ MODKEY|ShiftMask,	XK_0,		replacetag,	{ .i = Tfnord } }, \
	{ MODKEY|ShiftMask,	XK_1,		replacetag,	{ .i = Tdev } }, \
	{ MODKEY|ShiftMask,	XK_2,		replacetag,	{ .i = Tnet } }, \
	{ MODKEY|ShiftMask,	XK_3,		replacetag,	{ .i = Twork } }, \
	{ MODKEY|ShiftMask,	XK_4,		replacetag,	{ .i = Tmisc } }, \
	{ MODKEY|ShiftMask,	XK_c,		killclient,	{ 0 } }, \
	{ MODKEY|ShiftMask,	XK_q,		quit,		{ 0 } }, \
	{ MODKEY|ShiftMask,	XK_Return,	spawn,		{ .argv = term } }, \
	{ MODKEY|ShiftMask,	XK_g,		spawn,		{ .argv = gimp } }, \
	{ MODKEY|ShiftMask,	XK_l,		spawn,		{ .argv = xlock } }, \
	{ MODKEY|ShiftMask,	XK_w,		spawn,		{ .argv = browse } }, \
};

#define RULES \
static Rule rule[] = { \
	/* class:instance	tags				isfloat */ \
	{ "Firefox.*",		{ [Tnet] = "net" },		False }, \
	{ "Gimp.*",		{ 0 },				True}, \
};

#define ARRANGE dotile
