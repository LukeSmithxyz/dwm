/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#define FONT			"fixed"
#define BGCOLOR			"#666699"
#define FGCOLOR			"#eeeeee"
#define BORDERCOLOR		"#9999CC"
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
	const char *term[] = { "xterm", NULL };

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
};

#define RULES \
static Rule rule[] = { \
	/* class:instance	tags				isfloat */ \
	{ "Firefox.*",		{ [Tnet] = "net" },		False }, \
	{ "Gimp.*",		{ 0 },				True}, \
};

#define ARRANGE dotile
