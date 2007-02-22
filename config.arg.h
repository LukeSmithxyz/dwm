/* (C)opyright MMVI-MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

/* appearance */
#define BORDERPX		1
#define FONT			"-*-proggyclean-medium-r-*-*-13-*-*-*-*-*-*-*"
#define NORMBORDERCOLOR		"#333"
#define NORMBGCOLOR		"#222"
#define NORMFGCOLOR		"#ccc"
#define SELBORDERCOLOR		"#8c8"
#define SELBGCOLOR		"#555"
#define SELFGCOLOR		"#fff"
#define TOPBAR			True		/* False */

/* behavior */
#define TAGS \
const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL };
#define RULES \
static Rule rule[] = { \
	/* class:instance:title regex	tags regex	isuntiled */ \
	{ "Firefox",			"3",		False }, \
	{ "Gimp",			NULL,		True }, \
	{ "MPlayer",			NULL,		True }, \
	{ "Acroread",			NULL,		True }, \
};

/* layout(s) */
#define LAYOUTS \
static Layout layout[] = { \
	/* symbol		function */ \
	{ "[]=",		tile }, /* first entry is default */ \
	{ "><>",		untile }, \
};
#define MASTERWIDTH		600		/* master width per thousand */
#define NMASTER			1		/* clients in master area */
#define SNAP			32		/* snap pixel */

/* key definitions */
#define MODKEY			Mod1Mask
#define KEYS \
static Key key[] = { \
	/* modifier			key		function	argument */ \
	{ MODKEY,			XK_p,		spawn, \
		"exe=\"$(lsx `echo $PATH | sed 's/:/ /g'` | sort -u " \
		" | dmenu -fn '"FONT"' -nb '"NORMBGCOLOR"' -nf '"NORMFGCOLOR"' " \
		"-sb '"SELBGCOLOR"' -sf '"SELFGCOLOR"')\" && exec $exe" }, \
	{ MODKEY|ShiftMask,		XK_Return,	spawn, \
		"exec urxvtcd -tr -bg '#292929' -fg '#eee' -cr '#eee' +sb -fn '"FONT"'" }, \
	{ MODKEY,			XK_space,	setlayout,	NULL }, \
	{ MODKEY,			XK_d,		incnmaster,	"-1" }, \
	{ MODKEY,			XK_i,		incnmaster,	"1" }, \
	{ MODKEY,			XK_h,		incmasterw,	"-32" }, \
	{ MODKEY,			XK_l,		incmasterw,	"32" }, \
	{ MODKEY,			XK_j,		focusclient,	"1" }, \
	{ MODKEY,			XK_k,		focusclient,	"-1" }, \
	{ MODKEY,			XK_m,		togglemax,	NULL }, \
	{ MODKEY,			XK_Return,	zoom,		NULL }, \
	{ MODKEY|ShiftMask,		XK_space,	toggletiled,	NULL }, \
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
