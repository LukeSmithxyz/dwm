/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#define TAGS \
const char *tags[] = { "work", "net", "fnord", NULL };

#define DEFMODE			dotile /* dofloat */
#define DEFTAG			0 /* index */
#define FONT			"-*-terminus-medium-*-*-*-12-*-*-*-*-*-iso10646-*"
#define BGCOLOR			"#0d121d"
#define FGCOLOR			"#eeeeee"
#define BORDERCOLOR		"#3f484d"
#define MODKEY			Mod1Mask
#define MASTERW			60 /* percent */

#define KEYS \
static Key key[] = { \
	/* modifier			key		function	arguments */ \
	{ MODKEY,			XK_1,		view,		{ .i = 0 } }, \
	{ MODKEY,			XK_2,		view,		{ .i = 1 } }, \
	{ MODKEY,			XK_3,		view,		{ .i = 2 } }, \
	{ MODKEY,			XK_j,		focusnext,	{ 0 } }, \
	{ MODKEY,			XK_k,		focusprev,	{ 0 } }, \
	{ MODKEY,			XK_m,		togglemax,	{ 0 } }, \
	{ MODKEY,			XK_p,		spawn, \
		{ .cmd = "exec `ls -lL /usr/bin /usr/local/bin 2>/dev/null | " \
			"awk 'NF>2 && $1 ~ /^[^d].*x/ {print $NF}' | sort | uniq | dmenu`" } }, \
	{ MODKEY,			XK_space,	togglemode,	{ 0 } }, \
	{ MODKEY,			XK_Return,	zoom,		{ 0 } }, \
	{ MODKEY|ControlMask,		XK_1,		toggleview,	{ .i = 0 } }, \
	{ MODKEY|ControlMask,		XK_2,		toggleview,	{ .i = 1 } }, \
	{ MODKEY|ControlMask,		XK_3,		toggleview,	{ .i = 2 } }, \
	{ MODKEY|ShiftMask,		XK_1,		tag,		{ .i = 0 } }, \
	{ MODKEY|ShiftMask,		XK_2,		tag,		{ .i = 1 } }, \
	{ MODKEY|ShiftMask,		XK_3,		tag,		{ .i = 2 } }, \
	{ MODKEY|ShiftMask,		XK_c,		killclient,	{ 0 } }, \
	{ MODKEY|ShiftMask,		XK_q,		quit,		{ 0 } }, \
	{ MODKEY|ShiftMask,		XK_Return,	spawn, \
		{ .cmd = "exec urxvt +sb -tr -bg black -fg '#eeeeee' -cr '#eeeeee' +sb " \
			"-fn '-*-terminus-medium-*-*-*-12-*-*-*-*-*-iso10646-*'" } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_1,		toggletag,	{ .i = 0 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_2,		toggletag,	{ .i = 1 } }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_3,		toggletag,	{ .i = 2 } }, \
};

#define RULES \
static Rule rule[] = { \
	/* class:instance regex		tags regex	isfloat */ \
	{ "Firefox.*",			"net",		False }, \
	{ "Gimp.*",			NULL,		True}, \
	{ "MPlayer.*",			NULL,		True}, \
	{ "Acroread.*",			NULL,		True}, \
};
