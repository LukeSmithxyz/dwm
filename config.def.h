/* See LICENSE file for copyright and license details. */

/* appearance */
#define FONT            "-*-terminus-medium-r-normal-*-14-*-*-*-*-*-*-*"
#define NORMBORDERCOLOR "#cccccc"
#define NORMBGCOLOR     "#cccccc"
#define NORMFGCOLOR     "#000000"
#define SELBORDERCOLOR  "#0066ff"
#define SELBGCOLOR      "#0066ff"
#define SELFGCOLOR      "#ffffff"
static uint borderpx  = 1;        /* border pixel of windows */
static uint snap      = 32;       /* snap pixel */
static Bool showbar   = True;     /* False means no bar */
static Bool topbar    = True;     /* False means bottom bar */

/* tagging */
static const char tags[][MAXTAGLEN] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static Rule rules[] = {
	/* class      instance    title       tags ref      isfloating */
	{ "Gimp",     NULL,       NULL,       0,            True },
	{ "Firefox",  NULL,       NULL,       1 << 8,       True },
};

/* layout(s) */
static float mfact           = 0.55;
static Bool resizehints       = False;     /* False means respect size hints in tiled resizals */

static Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile }, /* first entry is default */
	{ "><>",      NULL }, /* no layout function means floating behavior */
};

/* key definitions */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
        { MODKEY,                       KEY,      view,           TAG }, \
        { MODKEY|ControlMask,           KEY,      toggleview,     TAG }, \
        { MODKEY|ShiftMask,             KEY,      tag,            TAG }, \
        { MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      TAG },

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = (char *[]){"dmenu_run", "-fn", FONT, "-nb", NORMBGCOLOR, "-nf", NORMFGCOLOR, "-sb", SELBGCOLOR, "-sf", SELFGCOLOR, NULL}} },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = (char *[]){"uxterm", NULL}} },
	{ MODKEY,                       XK_b,      togglebar,      {0}},
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1  }},
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1  }},
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05}},
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05}},
	{ MODKEY,                       XK_m,      togglemax,      {0}},
	{ MODKEY,                       XK_Return, zoom,           {0}},
	{ MODKEY,                       XK_Tab,    view,           {0}},
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0}},
	{ MODKEY,                       XK_space,  togglelayout,   {0}},
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0}},
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	TAGKEYS(                        XK_1,                      {.ui = 1 << 0} )
	TAGKEYS(                        XK_2,                      {.ui = 1 << 1} )
	TAGKEYS(                        XK_3,                      {.ui = 1 << 2} )
	TAGKEYS(                        XK_4,                      {.ui = 1 << 3} )
	TAGKEYS(                        XK_5,                      {.ui = 1 << 4} )
	TAGKEYS(                        XK_6,                      {.ui = 1 << 5} )
	TAGKEYS(                        XK_7,                      {.ui = 1 << 6} )
	TAGKEYS(                        XK_8,                      {.ui = 1 << 7} )
	TAGKEYS(                        XK_9,                      {.ui = 1 << 8} )
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0}},
};
