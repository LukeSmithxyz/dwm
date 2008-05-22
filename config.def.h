/* See LICENSE file for copyright and license details. */

/* appearance */
#define FONT            "-*-terminus-medium-r-normal-*-14-*-*-*-*-*-*-*"
#define NORMBORDERCOLOR "#cccccc"
#define NORMBGCOLOR     "#cccccc"
#define NORMFGCOLOR     "#000000"
#define SELBORDERCOLOR  "#0066ff"
#define SELBGCOLOR      "#0066ff"
#define SELFGCOLOR      "#ffffff"
unsigned int borderpx  = 1;        /* border pixel of windows */
unsigned int snap      = 32;       /* snap pixel */
Bool showbar           = True;     /* False means no bar */
Bool topbar            = True;     /* False means bottom bar */

/* tagging */
const char tags[][MAXTAGLEN] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

Rule rules[] = {
	/* class      instance    title       tags ref      isfloating */
	{ "Gimp",     NULL,       NULL,       0,            True },
	{ "Firefox",  NULL,       NULL,       1 << 8,       True },
};

/* layout(s) */
double mfact           = 0.55;
Bool resizehints       = True;     /* False means respect size hints in tiled resizals */

Layout layouts[] = {
	/* symbol     arrange  geom */
	{ "[]=",      tile,    updatetilegeom }, /* first entry is default */
	{ "><>",      NULL,    NULL           }, /* no layout function means floating behavior */
};

/* key definitions */
#define MODKEY Mod1Mask
Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          (char *)"exec dmenu_run -fn '"FONT"' -nb '"NORMBGCOLOR"' -nf '"NORMFGCOLOR"' -sb '"SELBGCOLOR"' -sf '"SELFGCOLOR"'" },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          (char *)"exec uxterm" },
	{ MODKEY,                       XK_b,      togglebar,      NULL },
	{ MODKEY,                       XK_j,      focusnext,      NULL },
	{ MODKEY,                       XK_k,      focusprev,      NULL },
	{ MODKEY,                       XK_h,      setmfact,       (double[]){+0.05} },
	{ MODKEY,                       XK_l,      setmfact,       (double[]){-0.05} },
	{ MODKEY,                       XK_Return, zoom,           NULL },
	{ MODKEY,                       XK_Tab,    viewprevtag,    NULL },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     NULL },
	{ MODKEY,                       XK_space,  togglelayout,   NULL },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, NULL },
	{ MODKEY,                       XK_0,      view,           (int[]){ ~0 } },
	{ MODKEY,                       XK_1,      view,           (int[]){ 1 << 0 } },
	{ MODKEY,                       XK_2,      view,           (int[]){ 1 << 1 } },
	{ MODKEY,                       XK_3,      view,           (int[]){ 1 << 2 } },
	{ MODKEY,                       XK_4,      view,           (int[]){ 1 << 3 } },
	{ MODKEY,                       XK_5,      view,           (int[]){ 1 << 4 } },
	{ MODKEY,                       XK_6,      view,           (int[]){ 1 << 5 } },
	{ MODKEY,                       XK_7,      view,           (int[]){ 1 << 6 } },
	{ MODKEY,                       XK_8,      view,           (int[]){ 1 << 7 } },
	{ MODKEY,                       XK_9,      view,           (int[]){ 1 << 8 } },
	{ MODKEY|ControlMask,           XK_1,      toggleview,     (int[]){ 1 << 0 } },
	{ MODKEY|ControlMask,           XK_2,      toggleview,     (int[]){ 1 << 1 } },
	{ MODKEY|ControlMask,           XK_3,      toggleview,     (int[]){ 1 << 2 } },
	{ MODKEY|ControlMask,           XK_4,      toggleview,     (int[]){ 1 << 3 } },
	{ MODKEY|ControlMask,           XK_5,      toggleview,     (int[]){ 1 << 4 } },
	{ MODKEY|ControlMask,           XK_6,      toggleview,     (int[]){ 1 << 5 } },
	{ MODKEY|ControlMask,           XK_7,      toggleview,     (int[]){ 1 << 6 } },
	{ MODKEY|ControlMask,           XK_8,      toggleview,     (int[]){ 1 << 7 } },
	{ MODKEY|ControlMask,           XK_9,      toggleview,     (int[]){ 1 << 8 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            (int[]){ ~0 } },
	{ MODKEY|ShiftMask,             XK_1,      tag,            (int[]){ 1 << 0 } },
	{ MODKEY|ShiftMask,             XK_2,      tag,            (int[]){ 1 << 1 } },
	{ MODKEY|ShiftMask,             XK_3,      tag,            (int[]){ 1 << 2 } },
	{ MODKEY|ShiftMask,             XK_4,      tag,            (int[]){ 1 << 3 } },
	{ MODKEY|ShiftMask,             XK_5,      tag,            (int[]){ 1 << 4 } },
	{ MODKEY|ShiftMask,             XK_6,      tag,            (int[]){ 1 << 5 } },
	{ MODKEY|ShiftMask,             XK_7,      tag,            (int[]){ 1 << 6 } },
	{ MODKEY|ShiftMask,             XK_8,      tag,            (int[]){ 1 << 7 } },
	{ MODKEY|ShiftMask,             XK_9,      tag,            (int[]){ 1 << 8 } },
	{ MODKEY|ControlMask|ShiftMask, XK_1,      toggletag,      (int[]){ 1 << 0 } },
	{ MODKEY|ControlMask|ShiftMask, XK_2,      toggletag,      (int[]){ 1 << 1 } },
	{ MODKEY|ControlMask|ShiftMask, XK_3,      toggletag,      (int[]){ 1 << 2 } },
	{ MODKEY|ControlMask|ShiftMask, XK_4,      toggletag,      (int[]){ 1 << 3 } },
	{ MODKEY|ControlMask|ShiftMask, XK_5,      toggletag,      (int[]){ 1 << 4 } },
	{ MODKEY|ControlMask|ShiftMask, XK_6,      toggletag,      (int[]){ 1 << 5 } },
	{ MODKEY|ControlMask|ShiftMask, XK_7,      toggletag,      (int[]){ 1 << 6 } },
	{ MODKEY|ControlMask|ShiftMask, XK_8,      toggletag,      (int[]){ 1 << 7 } },
	{ MODKEY|ControlMask|ShiftMask, XK_9,      toggletag,      (int[]){ 1 << 8 } },
	{ MODKEY|ShiftMask,             XK_q,      quit,           NULL },
};
