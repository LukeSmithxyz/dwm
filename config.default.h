/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#define TAGS \
const char *tags[] = { "1", "2", "3", "4", "5", NULL };

#define DEFMODE			dotile /* dofloat */
#define FLOATSYMBOL		"><>"
#define STACKPOS		StackRight	/* StackLeft */
#define TILESYMBOL		"[]="

#define FONT			"fixed"
#define NORMBGCOLOR		"#333366"
#define NORMFGCOLOR		"#cccccc"
#define SELBGCOLOR		"#666699"
#define SELFGCOLOR		"#eeeeee"
#define STATUSBGCOLOR		"#dddddd"
#define STATUSFGCOLOR		"#222222"

#define MASTER			60 /* percent */
#define MODKEY			Mod1Mask

#define KEYS \
static Key key[] = { \
	/* modifier			key		function	arguments */ \
	{ MODKEY|ShiftMask,		XK_Return,	spawn,		{ .cmd = "exec xterm" } }, \
	{ MODKEY,			XK_Tab,		focusnext,	{ 0 } }, \
	{ MODKEY|ShiftMask,		XK_Tab,		focusprev,	{ 0 } }, \
	{ MODKEY,			XK_Return,	zoom,		{ 0 } }, \
	{ MODKEY,			XK_b,		togglestackpos,	{ 0 } }, \
	{ MODKEY,			XK_g,		resizecol,	{ .i = 20 } }, \
	{ MODKEY,			XK_s,		resizecol,	{ .i = -20 } }, \
	{ MODKEY|ShiftMask,		XK_1,		tag,		{ .i = 0 } }, \
	{ MODKEY|ShiftMask,		XK_2,		tag,		{ .i = 1 } }, \
	{ MODKEY|ShiftMask,		XK_3,		tag,		{ .i = 2 } }, \
	{ MODKEY|ShiftMask,		XK_4,		tag,		{ .i = 3 } }, \
	{ MODKEY|ShiftMask,		XK_5,		tag,		{ .i = 4 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_1,		toggletag,	{ .i = 0 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_2,		toggletag,	{ .i = 1 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_3,		toggletag,	{ .i = 2 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_4,		toggletag,	{ .i = 3 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_5,		toggletag,	{ .i = 4 } }, \
	{ MODKEY|ShiftMask,		XK_c,		killclient,	{ 0 } }, \
	{ MODKEY,			XK_space,	togglemode,	{ 0 } }, \
	{ MODKEY,			XK_0,		viewall,	{ 0 } }, \
	{ MODKEY,			XK_1,		view,		{ .i = 0 } }, \
	{ MODKEY,			XK_2,		view,		{ .i = 1 } }, \
	{ MODKEY,			XK_3,		view,		{ .i = 2 } }, \
	{ MODKEY,			XK_4,		view,		{ .i = 3 } }, \
	{ MODKEY,			XK_5,		view,		{ .i = 4 } }, \
	{ MODKEY|ControlMask,		XK_1,		toggleview,	{ .i = 0 } }, \
	{ MODKEY|ControlMask,		XK_2,		toggleview,	{ .i = 1 } }, \
	{ MODKEY|ControlMask,		XK_3,		toggleview,	{ .i = 2 } }, \
	{ MODKEY|ControlMask,		XK_4,		toggleview,	{ .i = 3 } }, \
	{ MODKEY|ControlMask,		XK_5,		toggleview,	{ .i = 4 } }, \
	{ MODKEY|ShiftMask,		XK_q,		quit,		{ 0 } }, \
};

/* Query class:instance:title for regex matching info with following command:
 * xprop | awk -F '"' '/^WM_CLASS/ { printf("%s:%s:",$4,$2) }; /^WM_NAME/ { printf("%s\n",$2) }' */
#define RULES \
static Rule rule[] = { \
	/* class:instance:title regex	tags regex	isfloat */ \
	{ "Firefox.*",			"2",		False }, \
	{ "Gimp.*",			NULL,		True}, \
};
