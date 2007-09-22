/* See LICENSE file for copyright and license details. */

/* appearance */
#define BARPOS			BarTop /* BarBot, BarOff */
#define BORDERPX		1
#define FONT			"-*-terminus-medium-r-*-*-12-*-*-*-*-*-iso10646-*"
#define NORMBORDERCOLOR		"#333"
#define NORMBGCOLOR		"#000"
#define NORMFGCOLOR		"#ccc"
#define SELBORDERCOLOR		"#f00"
#define SELBGCOLOR		"#00f"
#define SELFGCOLOR		"#fff"

/* tagging */
const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL };
Rule rules[] = {
	/* class:instance:title regex	tags regex	isfloating */
	{ "Firefox",			"3",		False },
	{ "Gimp",			NULL,		True },
	{ "MPlayer",			NULL,		True },
	{ "Acroread",			NULL,		True },
};

/* layout(s) */
#define ISTILE			isarrange(tile)
#define MWFACT			0.6	/* master width factor [0.1 .. 0.9] */
#define RESIZEHINTS		True	/* False - respect size hints in tiled resizals */
#define SNAP			32	/* snap pixel */
Layout layouts[] = {
	/* symbol		function */
	{ "[]=",		tile }, /* first entry is default */
	{ "><>",		floating },
};

/* key definitions */
#define MODKEY			Mod1Mask
#define KEYS \
Key keys[] = { \
	/* modifier			key		function	argument */ \
	{ MODKEY,			XK_p,		spawn, \
		"exe=`dmenu_path | dmenu -fn '"FONT"' -nb '"NORMBGCOLOR"' -nf '"NORMFGCOLOR"'" \
		" -sb '"SELBGCOLOR"' -sf '"SELFGCOLOR"'` && exec $exe" }, \
	{ MODKEY|ShiftMask,		XK_Return,	spawn, \
		"exec xterm -bg '"NORMBGCOLOR"' -fg '"NORMFGCOLOR"' -cr '"NORMFGCOLOR"' +sb -fn '"FONT"'" }, \
	{ MODKEY,			XK_space,	setlayout,	NULL }, \
	{ MODKEY,			XK_b,		togglebar,	NULL }, \
	{ MODKEY,			XK_j,		focusnext,	NULL }, \
	{ MODKEY,			XK_k,		focusprev,	NULL }, \
	{ MODKEY,			XK_h,		setmwfact,	"-0.05" }, \
	{ MODKEY,			XK_l,		setmwfact,	"+0.05" }, \
	{ MODKEY,			XK_m,		togglemax,	NULL }, \
	{ MODKEY,			XK_Return,	zoom,		NULL }, \
	{ MODKEY|ShiftMask,		XK_space,	togglefloating,	NULL }, \
	{ MODKEY|ShiftMask,		XK_c,		killclient,	NULL }, \
	{ MODKEY,			XK_0,		view,		NULL }, \
	{ MODKEY,			XK_1,		view,		tags[0] }, \
	{ MODKEY,			XK_2,		view,		tags[1] }, \
	{ MODKEY,			XK_3,		view,		tags[2] }, \
	{ MODKEY,			XK_4,		view,		tags[3] }, \
	{ MODKEY,			XK_5,		view,		tags[4] }, \
	{ MODKEY,			XK_6,		view,		tags[5] }, \
	{ MODKEY,			XK_7,		view,		tags[6] }, \
	{ MODKEY,			XK_8,		view,		tags[7] }, \
	{ MODKEY,			XK_9,		view,		tags[8] }, \
	{ MODKEY|ControlMask,		XK_1,		toggleview,	tags[0] }, \
	{ MODKEY|ControlMask,		XK_2,		toggleview,	tags[1] }, \
	{ MODKEY|ControlMask,		XK_3,		toggleview,	tags[2] }, \
	{ MODKEY|ControlMask,		XK_4,		toggleview,	tags[3] }, \
	{ MODKEY|ControlMask,		XK_5,		toggleview,	tags[4] }, \
	{ MODKEY|ControlMask,		XK_6,		toggleview,	tags[5] }, \
	{ MODKEY|ControlMask,		XK_7,		toggleview,	tags[6] }, \
	{ MODKEY|ControlMask,		XK_8,		toggleview,	tags[7] }, \
	{ MODKEY|ControlMask,		XK_9,		toggleview,	tags[8] }, \
	{ MODKEY|ShiftMask,		XK_0,		tag,		NULL }, \
	{ MODKEY|ShiftMask,		XK_1,		tag,		tags[0] }, \
	{ MODKEY|ShiftMask,		XK_2,		tag,		tags[1] }, \
	{ MODKEY|ShiftMask,		XK_3,		tag,		tags[2] }, \
	{ MODKEY|ShiftMask,		XK_4,		tag,		tags[3] }, \
	{ MODKEY|ShiftMask,		XK_5,		tag,		tags[4] }, \
	{ MODKEY|ShiftMask,		XK_6,		tag,		tags[5] }, \
	{ MODKEY|ShiftMask,		XK_7,		tag,		tags[6] }, \
	{ MODKEY|ShiftMask,		XK_8,		tag,		tags[7] }, \
	{ MODKEY|ShiftMask,		XK_9,		tag,		tags[8] }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_1,		toggletag,	tags[0] }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_2,		toggletag,	tags[1] }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_3,		toggletag,	tags[2] }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_4,		toggletag,	tags[3] }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_5,		toggletag,	tags[4] }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_6,		toggletag,	tags[5] }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_7,		toggletag,	tags[6] }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_8,		toggletag,	tags[7] }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_9,		toggletag,	tags[8] }, \
	{ MODKEY|ShiftMask,		XK_q,		quit,		NULL }, \
};
