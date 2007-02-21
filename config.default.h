/* (C)opyright MMVI-MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

/* appearance */
#define BORDERPX		1
#define FONT			"-*-fixed-medium-r-normal-*-13-*-*-*-*-*-*-*"
#define NORMBORDERCOLOR		"#dddddd"
#define NORMBGCOLOR		"#eeeeee"
#define NORMFGCOLOR		"#222222"
#define SELBORDERCOLOR		"#ff0000"
#define SELBGCOLOR		"#006699"
#define SELFGCOLOR		"#ffffff"
#define TOPBAR			True		/* False */

/* behavior */
#define SNAP			20		/* pixel */
#define TAGS \
const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL };
/* Query class:instance:title for regex matching info with following command:
 * xprop | awk -F '"' '/^WM_CLASS/ { printf("%s:%s:",$4,$2) }; /^WM_NAME/ { printf("%s\n",$2) }' */
#define RULES \
static Rule rule[] = { \
	/* class:instance:title regex	tags regex	isversatile */ \
	{ "Gimp",			NULL,		True }, \
	{ "MPlayer",			NULL,		True }, \
	{ "Acroread",			NULL,		True }, \
};

/* layout(s) */
#define LAYOUTS \
static Layout layout[] = { \
	/* symbol		function */ \
	{ "[]=",		tile }, /* first entry is default */ \
	{ "><>",		versatile }, \
};
#define MASTER			600		/* per thousand */
#define NMASTER			1		/* clients in master area */

/* key definitions */
#define MODKEY			Mod1Mask
#define KEYS \
static Key key[] = { \
	/* modifier			key		function	argument */ \
	{ MODKEY|ShiftMask,		XK_Return,	spawn,		{ .cmd = "exec xterm" } }, \
	{ MODKEY,			XK_Tab,		focusnext,	{ 0 } }, \
	{ MODKEY|ShiftMask,		XK_Tab,		focusprev,	{ 0 } }, \
	{ MODKEY,			XK_Return,	zoom,		{ 0 } }, \
	{ MODKEY,			XK_g,		resizemaster,	{ .i = 15 } }, \
	{ MODKEY,			XK_s,		resizemaster,	{ .i = -15 } }, \
	{ MODKEY,			XK_i,		incnmaster,	{ .i = 1 } }, \
	{ MODKEY,			XK_d,		incnmaster,	{ .i = -1 } }, \
	{ MODKEY|ShiftMask,		XK_0,		tag,		{ .i = -1 } }, \
	{ MODKEY|ShiftMask,		XK_1,		tag,		{ .i = 0 } }, \
	{ MODKEY|ShiftMask,		XK_2,		tag,		{ .i = 1 } }, \
	{ MODKEY|ShiftMask,		XK_3,		tag,		{ .i = 2 } }, \
	{ MODKEY|ShiftMask,		XK_4,		tag,		{ .i = 3 } }, \
	{ MODKEY|ShiftMask,		XK_5,		tag,		{ .i = 4 } }, \
	{ MODKEY|ShiftMask,		XK_6,		tag,		{ .i = 5 } }, \
	{ MODKEY|ShiftMask,		XK_7,		tag,		{ .i = 6 } }, \
	{ MODKEY|ShiftMask,		XK_8,		tag,		{ .i = 7 } }, \
	{ MODKEY|ShiftMask,		XK_9,		tag,		{ .i = 8 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_1,		toggletag,	{ .i = 0 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_2,		toggletag,	{ .i = 1 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_3,		toggletag,	{ .i = 2 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_4,		toggletag,	{ .i = 3 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_5,		toggletag,	{ .i = 4 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_6,		toggletag,	{ .i = 5 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_7,		toggletag,	{ .i = 6 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_8,		toggletag,	{ .i = 7 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_9,		toggletag,	{ .i = 8 } }, \
	{ MODKEY|ShiftMask,		XK_c,		killclient,	{ 0 } }, \
	{ MODKEY,			XK_space,	setlayout,	{ .i = -1 } }, \
	{ MODKEY|ShiftMask,		XK_space,	toggleversatile,{ 0 } }, \
	{ MODKEY,			XK_0,		view,		{ .i = -1 } }, \
	{ MODKEY,			XK_1,		view,		{ .i = 0 } }, \
	{ MODKEY,			XK_2,		view,		{ .i = 1 } }, \
	{ MODKEY,			XK_3,		view,		{ .i = 2 } }, \
	{ MODKEY,			XK_4,		view,		{ .i = 3 } }, \
	{ MODKEY,			XK_5,		view,		{ .i = 4 } }, \
	{ MODKEY,			XK_6,		view,		{ .i = 5 } }, \
	{ MODKEY,			XK_7,		view,		{ .i = 6 } }, \
	{ MODKEY,			XK_8,		view,		{ .i = 7 } }, \
	{ MODKEY,			XK_9,		view,		{ .i = 8 } }, \
	{ MODKEY|ControlMask,		XK_1,		toggleview,	{ .i = 0 } }, \
	{ MODKEY|ControlMask,		XK_2,		toggleview,	{ .i = 1 } }, \
	{ MODKEY|ControlMask,		XK_3,		toggleview,	{ .i = 2 } }, \
	{ MODKEY|ControlMask,		XK_4,		toggleview,	{ .i = 3 } }, \
	{ MODKEY|ControlMask,		XK_5,		toggleview,	{ .i = 4 } }, \
	{ MODKEY|ControlMask,		XK_6,		toggleview,	{ .i = 5 } }, \
	{ MODKEY|ControlMask,		XK_7,		toggleview,	{ .i = 6 } }, \
	{ MODKEY|ControlMask,		XK_8,		toggleview,	{ .i = 7 } }, \
	{ MODKEY|ControlMask,		XK_9,		toggleview,	{ .i = 8 } }, \
	{ MODKEY|ShiftMask,		XK_q,		quit,		{ 0 } }, \
};
