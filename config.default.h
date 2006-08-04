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
	/* modifier		key		function	arguments */ \
	{ MODKEY,		XK_0,		view,		{ .i = 0 } }, \
	{ MODKEY,		XK_1,		view,		{ .i = 1 } }, \
	{ MODKEY,		XK_2,		view,		{ .i = 2 } }, \
	{ MODKEY,		XK_3,		view,		{ .i = 3 } }, \
	{ MODKEY,		XK_4,		view,		{ .i = 4 } }, \
	{ MODKEY,		XK_h,		viewprev,	{ 0 } }, \
	{ MODKEY,		XK_j,		focusnext,	{ 0 } }, \
	{ MODKEY,		XK_k,		focusprev,	{ 0 } }, \
	{ MODKEY,		XK_l,		viewnext,	{ 0 } }, \
	{ MODKEY,		XK_m,		togglemax,	{ 0 } }, \
	{ MODKEY,		XK_space,	togglemode,	{ 0 } }, \
	{ MODKEY,		XK_Return,	zoom,		{ 0 } }, \
	{ MODKEY|ControlMask,	XK_0,		appendtag,	{ .i = 0 } }, \
	{ MODKEY|ControlMask,	XK_1,		appendtag,	{ .i = 1 } }, \
	{ MODKEY|ControlMask,	XK_2,		appendtag,	{ .i = 2 } }, \
	{ MODKEY|ControlMask,	XK_3,		appendtag,	{ .i = 3 } }, \
	{ MODKEY|ControlMask,	XK_4,		appendtag,	{ .i = 4 } }, \
	{ MODKEY|ShiftMask,	XK_0,		replacetag,	{ .i = 0 } }, \
	{ MODKEY|ShiftMask,	XK_1,		replacetag,	{ .i = 1 } }, \
	{ MODKEY|ShiftMask,	XK_2,		replacetag,	{ .i = 2 } }, \
	{ MODKEY|ShiftMask,	XK_3,		replacetag,	{ .i = 3 } }, \
	{ MODKEY|ShiftMask,	XK_4,		replacetag,	{ .i = 4 } }, \
	{ MODKEY|ShiftMask,	XK_c,		killclient,	{ 0 } }, \
	{ MODKEY|ShiftMask,	XK_q,		quit,		{ 0 } }, \
	/* { MODKEY|ShiftMask,	XK_x,		spawn, */ \
	/*	{ .cmd = "exec `ls -lL /usr/bin /usr/local/bin 2>/dev/null |" */ \
	/*	" awk 'NF>2 && $1 ~ /^[^d].*x/ {print $NF}' | sort | uniq | dmenu`" } }, */ \
	{ MODKEY|ShiftMask,	XK_Return,	spawn,		{ .cmd = "exec xterm" } }, \
};

#define RULES \
	const unsigned int two[] = { 2 }; \
static Rule rule[] = { \
	/* class:instance	tags		isfloat */ \
	{ "Firefox.*",		two,	False }, \
	{ "Gimp.*",		NULL,		True}, \
};
