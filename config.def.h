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
#define TAGKEYS(KEY,TAG) \
        { MODKEY,                       KEY,      view,           TAG }, \
        { MODKEY|ControlMask,           KEY,      toggleview,     TAG }, \
        { MODKEY|ShiftMask,             KEY,      tag,            TAG }, \
        { MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      TAG },

Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          (char *)"exec dmenu_run -fn '"FONT"' -nb '"NORMBGCOLOR"' -nf '"NORMFGCOLOR"' -sb '"SELBGCOLOR"' -sf '"SELFGCOLOR"'" },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          (char *)"exec uxterm" },
	{ MODKEY,                       XK_b,      togglebar,      NULL },
	{ MODKEY,                       XK_j,      focusnext,      NULL },
	{ MODKEY,                       XK_k,      focusprev,      NULL },
	{ MODKEY,                       XK_h,      setmfact,       (double[]){-0.05} },
	{ MODKEY,                       XK_l,      setmfact,       (double[]){+0.05} },
	{ MODKEY,                       XK_m,      togglemax,      NULL },
	{ MODKEY,                       XK_Return, zoom,           NULL },
	{ MODKEY,                       XK_Tab,    viewprevtag,    NULL },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     NULL },
	{ MODKEY,                       XK_space,  togglelayout,   NULL },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, NULL },
	{ MODKEY,                       XK_0,      view,           (uint[]){ ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            (uint[]){ ~0 } },
	TAGKEYS(                        XK_1,                      (uint[]){ 1 << 0} )
	TAGKEYS(                        XK_2,                      (uint[]){ 1 << 1} )
	TAGKEYS(                        XK_3,                      (uint[]){ 1 << 2} )
	TAGKEYS(                        XK_4,                      (uint[]){ 1 << 3} )
	TAGKEYS(                        XK_5,                      (uint[]){ 1 << 4} )
	TAGKEYS(                        XK_6,                      (uint[]){ 1 << 5} )
	TAGKEYS(                        XK_7,                      (uint[]){ 1 << 6} )
	TAGKEYS(                        XK_8,                      (uint[]){ 1 << 7} )
	TAGKEYS(                        XK_9,                      (uint[]){ 1 << 8} )
	{ MODKEY|ShiftMask,             XK_q,      quit,           NULL },
};
