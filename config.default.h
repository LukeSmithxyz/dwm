/* © 2006-2007 Anselm R. Garbe <garbeam at gmail dot com>
 * © 2006-2007 Sander van Dijk <a dot h dot vandijk at gmail dot com>
 * © 2007 Premysl Hruby <dfenze at gmail dot com>
 * See LICENSE file for license details. */

/* appearance */
#define BARPOS			BarTop /* BarBot, BarOff */
#define BORDERPX		1
#define FONT			"-*-fixed-medium-r-normal-*-13-*-*-*-*-*-*-*"
#define NORMBORDERCOLOR		"#dddddd"
#define NORMBGCOLOR		"#eeeeee"
#define NORMFGCOLOR		"#222222"
#define SELBORDERCOLOR		"#ff0000"
#define SELBGCOLOR		"#006699"
#define SELFGCOLOR		"#ffffff"

/* tagging */
#define TAGS \
const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL };
/* Query class:instance:title for regex matching info with following command:
 * xprop | awk -F '"' '/^WM_CLASS/ { printf("%s:%s:",$4,$2) }; /^WM_NAME/ { printf("%s\n",$2) }' */
#define RULES \
static Rule rule[] = { \
	/* class:instance:title regex	tags regex	isfloating */ \
	{ "Gimp",			NULL,		True }, \
	{ "MPlayer",			NULL,		True }, \
	{ "Acroread",			NULL,		True }, \
};

/* layout(s) */
#define LAYOUTS \
static Layout layout[] = { \
	/* symbol		function */ \
	{ "[]=",		tile }, /* first entry is default */ \
	{ "><>",		floating }, \
};
#define MASTERWIDTH		600		/* master width per thousand */
#define NMASTER			1		/* clients in master area */
#define SNAP			32		/* snap pixel */

/* key definitions */
#define MODKEY			Mod1Mask
#define KEYS \
static Key key[] = { \
	/* modifier			key		function	argument */ \
	{ MODKEY|ShiftMask,		XK_Return,	spawn,		"exec xterm" }, \
	{ MODKEY,			XK_p,		spawn, 		"exe=`dmenu_path | dmenu` && exec $exe" }, \
	{ MODKEY,			XK_space,	setlayout,	NULL }, \
	{ MODKEY,			XK_b,		togglebar,	NULL }, \
	{ MODKEY,			XK_h,		incmasterw,	"-32" }, \
	{ MODKEY,			XK_l,		incmasterw,	"32" }, \
	{ MODKEY|ShiftMask,		XK_j,		incnmaster,	"1" }, \
	{ MODKEY|ShiftMask,		XK_k,		incnmaster,	"-1" }, \
	{ MODKEY,			XK_j,		focusclient,	"1" }, \
	{ MODKEY,			XK_k,		focusclient,	"-1" }, \
	{ MODKEY,			XK_m,		togglemax,	NULL }, \
	{ MODKEY,			XK_Return,	zoom,		NULL }, \
	{ MODKEY|ShiftMask,		XK_space,	togglefloating,	NULL }, \
	{ MODKEY|ShiftMask,		XK_c,		killclient,	NULL }, \
	{ MODKEY,			XK_0,		view,		NULL }, \
	{ MODKEY,			XK_1,		view,		"0" }, \
	{ MODKEY,			XK_2,		view,		"1" }, \
	{ MODKEY,			XK_3,		view,		"2" }, \
	{ MODKEY,			XK_4,		view,		"3" }, \
	{ MODKEY,			XK_5,		view,		"4" }, \
	{ MODKEY,			XK_6,		view,		"5" }, \
	{ MODKEY,			XK_7,		view,		"6" }, \
	{ MODKEY,			XK_8,		view,		"7" }, \
	{ MODKEY,			XK_9,		view,		"8" }, \
	{ MODKEY|ControlMask,		XK_1,		toggleview,	"0" }, \
	{ MODKEY|ControlMask,		XK_2,		toggleview,	"1" }, \
	{ MODKEY|ControlMask,		XK_3,		toggleview,	"2" }, \
	{ MODKEY|ControlMask,		XK_4,		toggleview,	"3" }, \
	{ MODKEY|ControlMask,		XK_5,		toggleview,	"4" }, \
	{ MODKEY|ControlMask,		XK_6,		toggleview,	"5" }, \
	{ MODKEY|ControlMask,		XK_7,		toggleview,	"6" }, \
	{ MODKEY|ControlMask,		XK_8,		toggleview,	"7" }, \
	{ MODKEY|ControlMask,		XK_9,		toggleview,	"8" }, \
	{ MODKEY|ShiftMask,		XK_0,		tag,		NULL }, \
	{ MODKEY|ShiftMask,		XK_1,		tag,		"0" }, \
	{ MODKEY|ShiftMask,		XK_2,		tag,		"1" }, \
	{ MODKEY|ShiftMask,		XK_3,		tag,		"2" }, \
	{ MODKEY|ShiftMask,		XK_4,		tag,		"3" }, \
	{ MODKEY|ShiftMask,		XK_5,		tag,		"4" }, \
	{ MODKEY|ShiftMask,		XK_6,		tag,		"5" }, \
	{ MODKEY|ShiftMask,		XK_7,		tag,		"6" }, \
	{ MODKEY|ShiftMask,		XK_8,		tag,		"7" }, \
	{ MODKEY|ShiftMask,		XK_9,		tag,		"8" }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_1,		toggletag,	"0" }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_2,		toggletag,	"1" }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_3,		toggletag,	"2" }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_4,		toggletag,	"3" }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_5,		toggletag,	"4" }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_6,		toggletag,	"5" }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_7,		toggletag,	"6" }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_8,		toggletag,	"7" }, \
	{ MODKEY|ControlMask|ShiftMask,	XK_9,		toggletag,	"8" }, \
	{ MODKEY|ShiftMask,		XK_q,		quit,		NULL }, \
};
