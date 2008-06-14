/* See LICENSE file for copyright and license details. */

/* appearance */
static const char font[]            = "-*-terminus-medium-r-normal-*-14-*-*-*-*-*-*-*";
static const char normbordercolor[] = "#cccccc";
static const char normbgcolor[]     = "#cccccc";
static const char normfgcolor[]     = "#000000";
static const char selbordercolor[]  = "#0066ff";
static const char selbgcolor[]      = "#0066ff";
static const char selfgcolor[]      = "#ffffff";
static uint borderpx                = 1;        /* border pixel of windows */
static uint snap                    = 32;       /* snap pixel */
static Bool showbar                 = True;     /* False means no bar */
static Bool topbar                  = True;     /* False means bottom bar */

#ifdef XINERAMA
static uint xidx                    = 0;        /* Xinerama screen index to use */
#endif

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
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static const char *dmenucmd[] = { "dmenu_run", "-fn", font, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL };
static const char *termcmd[]  = { "uxterm", NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_m,      togglemax,      {0} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_space,  togglelayout,   {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
};

/* button definitions */
#define TAGBUTTONS(TAG) \
	{ TAG,                  0,              Button1,        view,           {.ui = 1 << TAG} }, \
	{ TAG,                  0,              Button3,        toggleview,     {.ui = 1 << TAG} }, \
	{ TAG,                  MODKEY,         Button1,        tag,            {.ui = 1 << TAG} }, \
	{ TAG,                  MODKEY,         Button3,        toggletag,      {.ui = 1 << TAG} },

/* click can be a tag number (starting at 0),
 * ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        togglelayout,   {0} },
	{ ClkLtSymbol,          0,              Button3,        togglemax,      {0} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkWinTitle,          0,              Button4,        focusstack,     {.i = +1 } },
	{ ClkWinTitle,          0,              Button5,        focusstack,     {.i = -1 } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkRootWin,           Button1Mask,    Button3,        spawn,          {.v = termcmd } },
	TAGBUTTONS(0)
	TAGBUTTONS(1)
	TAGBUTTONS(2)
	TAGBUTTONS(3)
	TAGBUTTONS(4)
	TAGBUTTONS(5)
	TAGBUTTONS(6)
	TAGBUTTONS(7)
	TAGBUTTONS(8)
};

