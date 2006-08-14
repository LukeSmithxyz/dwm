/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#define TAGS \
const char *tags[] = { "0", "1", "2", "3", "4", NULL };

#define DEFMODE			dotile /* dofloat */
#define DEFTAG			1 /* index */
#define FONT			"fixed"
#define BGCOLOR			"#666699"
#define FGCOLOR			"#eeeeee"
#define BORDERCOLOR		"#9999CC"
#define MODKEY			Mod1Mask
#define NUMLOCKMASK		Mod2Mask
#define MASTERW			60 /* percent */

#define KEYS \
static Key key[] = { \
	/* modifier			key		function	arguments */ \
	{ MODKEY,			XK_0,		view,		{ .i = 0 } }, \
	{ MODKEY,			XK_1,		view,		{ .i = 1 } }, \
	{ MODKEY,			XK_2,		view,		{ .i = 2 } }, \
	{ MODKEY,			XK_3,		view,		{ .i = 3 } }, \
	{ MODKEY,			XK_4,		view,		{ .i = 4 } }, \
	{ MODKEY,			XK_j,		focusnext,	{ 0 } }, \
	{ MODKEY,			XK_k,		focusprev,	{ 0 } }, \
	{ MODKEY,			XK_m,		togglemax,	{ 0 } }, \
	{ MODKEY,			XK_space,	togglemode,	{ 0 } }, \
	{ MODKEY,			XK_Return,	zoom,		{ 0 } }, \
	{ MODKEY|ControlMask,		XK_0,		toggleview,	{ .i = 0 } }, \
	{ MODKEY|ControlMask,		XK_1,		toggleview,	{ .i = 1 } }, \
	{ MODKEY|ControlMask,		XK_2,		toggleview,	{ .i = 2 } }, \
	{ MODKEY|ControlMask,		XK_3,		toggleview,	{ .i = 3 } }, \
	{ MODKEY|ControlMask,		XK_4,		toggleview,	{ .i = 4 } }, \
	{ MODKEY|ShiftMask,		XK_0,		tag,		{ .i = 0 } }, \
	{ MODKEY|ShiftMask,		XK_1,		tag,		{ .i = 1 } }, \
	{ MODKEY|ShiftMask,		XK_2,		tag,		{ .i = 2 } }, \
	{ MODKEY|ShiftMask,		XK_3,		tag,		{ .i = 3 } }, \
	{ MODKEY|ShiftMask,		XK_4,		tag,		{ .i = 4 } }, \
	{ MODKEY|ShiftMask,		XK_c,		killclient,	{ 0 } }, \
	{ MODKEY|ShiftMask,		XK_q,		quit,		{ 0 } }, \
	{ MODKEY|ShiftMask,		XK_Return,	spawn,		{ .cmd = "exec xterm" } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_0,		toggletag,	{ .i = 0 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_1,		toggletag,	{ .i = 1 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_2,		toggletag,	{ .i = 2 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_3,		toggletag,	{ .i = 3 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_4,		toggletag,	{ .i = 4 } }, \
};

#define RULES \
static Rule rule[] = { \
	/* class:instance regex		tags regex	isfloat */ \
	{ "Firefox.*",			"2",		False }, \
	{ "Gimp.*",			NULL,		True}, \
};
