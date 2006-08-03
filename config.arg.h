/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#define TAGS \
const char *tags[] = { "fnord", "dev", "net", "work", "misc", NULL };

#define DEFMODE			dotile /* dofloat */
#define DEFTAG			1 /* index */
#define FONT			"-*-terminus-medium-*-*-*-13-*-*-*-*-*-iso10646-*"
#define BGCOLOR			"#0a2c2d"
#define FGCOLOR			"#ddeeee"
#define BORDERCOLOR		"#176164"
#define MODKEY			Mod1Mask
#define NUMLOCKMASK		Mod2Mask
#define MASTERW			60 /* percent */

#define KEYS \
	const char *browse[] = { "firefox", NULL }; \
	const char *gimp[] = { "gimp", NULL }; \
	const char *term[] = { \
		"urxvt", "-tr", "+sb", "-bg", "black", "-fg", "white", "-cr", "white", \
		"-fn", "-*-terminus-medium-*-*-*-13-*-*-*-*-*-iso10646-*", NULL \
	}; \
	const char *xlock[] = { "xlock", NULL }; \
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
	{ MODKEY|ShiftMask,	XK_4,		replacetag,	{ .i = 5 } }, \
	{ MODKEY|ShiftMask,	XK_c,		killclient,	{ 0 } }, \
	{ MODKEY|ShiftMask,	XK_q,		quit,		{ 0 } }, \
	{ MODKEY|ShiftMask,	XK_Return,	spawn,		{ .argv = term } }, \
	{ MODKEY|ShiftMask,	XK_g,		spawn,		{ .argv = gimp } }, \
	{ MODKEY|ShiftMask,	XK_l,		spawn,		{ .argv = xlock } }, \
	{ MODKEY|ShiftMask,	XK_w,		spawn,		{ .argv = browse } }, \
};

#define RULES \
	const unsigned int tag2[] = { 2 }; \
static Rule rule[] = { \
	/* class:instance	tags		isfloat */ \
	{ "Firefox.*",		tag2,		False }, \
	{ "Gimp.*",		NULL,		True}, \
	{ "MPlayer.*",		NULL,		True}, \
	{ "Acroread.*",		NULL,		True}, \
};
