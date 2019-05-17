/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "monospace:size=10" };
static const char dmenufont[]       = "monospace:size=10";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
static const unsigned int baralpha = 0xc0;
static const unsigned int borderalpha = OPAQUE;
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
};
static const unsigned int alphas[][3]      = {
	/*               fg      bg        border     */
	[SchemeNorm] = { OPAQUE, baralpha, borderalpha },
	[SchemeSel]  = { OPAQUE, baralpha, borderalpha },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           -1 },
	{ "Firefox",  NULL,       NULL,       1 << 8,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */

#include "layouts.c"
static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "🧱",      tile },    /* first entry is default */
	{ "☁",      NULL },    /* no layout function means floating behavior */
	{ "🔎",      monocle },
	{ "🌐",      grid },
	{ NULL,       NULL },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "st", NULL };

static const char *promptshutdown[]  = { "prompt","Are you sure you want to shutdown?","sudo -A shutdown -h now", NULL };
static const char *promptreboot[]  = { "prompt","Are you sure you want to reboot?","sudo -A reboot", NULL };
static const char *displayselect[]  = { "displayselect", NULL };
static const char *showreadme[]  = { "groff", "-mom", "~/.local/share/larbs/readme.mom", "-Tpdf","|", "zathura -", NULL };
static const char *mountdrives[]  = { "dmenumount",NULL };
static const char *unmountdrives[]  = { "dmenuumount",NULL };
static const char *musicplayer[] = {"st", "-e", "ncmpcpp", NULL };
static const char *filebrowser[] = {"st", "-e", "lf", NULL };
static const char *email[] = {"st", "-e", "neomutt", NULL };
static const char *rssreader[] = {"st", "-e", "newsboat", NULL };
static const char *systeminfo[] = {"st", "-e", "htop", NULL };
static const char *getunicode[]  = { "dmenuunicode", NULL };

/* Audio Commands */
static const char *pauseaudio[] = {"lmc", "toggle", NULL };
static const char *pauseall[] = {"pauseallmpv", NULL };
static const char *nexttrack[] = {"lmc", "next", NULL };
static const char *prevtrack[] = {"lmc", "prev", NULL };
static const char *restarttrack[] = {"lmc", "restart", NULL };
static const char *toggleaudio[] = {"lmc", "toggle", NULL };
static const char *seekfowardbig[] = {"lmc", "foward", "120", NULL };
static const char *seekbackwardbig[] = {"lmc", "back", "120", NULL };
static const char *seekfoward[] = {"lmc", "foward", "10", NULL };
static const char *seekbackward[] = {"lmc", "back", "10", NULL };
static const char *volup[] = {"lmc", "up", "5", NULL };
static const char *voldown[] = {"lmc", "down", "5", NULL };
static const char *volupbig[] = {"lmc", "up", "15", NULL };
static const char *voldownbig[] = {"lmc", "down", "15", NULL };

static const char *browser[] = {"$BROWSER", NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */

	/* audio */
	{ MODKEY,			XK_bracketright,spawn,		{.v = seekfoward} },
	{ MODKEY,			XK_bracketleft,	spawn,		{.v = seekbackward} },
	{ MODKEY|ShiftMask,		XK_bracketright,	spawn,		{.v = seekfowardbig} },
	{ MODKEY|ShiftMask,		XK_bracketleft,	spawn,		{.v = seekbackwardbig} },
	{ MODKEY,			XK_comma,	spawn,		{.v = prevtrack} },
	{ MODKEY,			XK_period,	spawn,		{.v = nexttrack} },
	{ MODKEY|ShiftMask,		XK_comma,	spawn,		{.v = restarttrack} },
	{ MODKEY,			XK_equal,	spawn,		{.v = volup} },
	{ MODKEY,			XK_minus,	spawn,		{.v = voldown} },
	{ MODKEY|ShiftMask,		XK_equal,	spawn,		{.v = volupbig} },
	{ MODKEY|ShiftMask,             XK_minus,	spawn,		{.v = voldownbig} },

	{ MODKEY|ShiftMask,		XK_w,		spawn,		{.v = browser } },

	{ MODKEY,			XK_F1,		spawn,		{.v = showreadme } },
	{ MODKEY,			XK_F3,		spawn,		{.v = displayselect } },
	{ MODKEY,			XK_F9,		spawn,		{.v = mountdrives } },
	{ MODKEY,			XK_F10,		spawn,		{.v = unmountdrives } },
	{ MODKEY,			XK_grave,	spawn,		{.v = getunicode } },

	{ MODKEY|ShiftMask,		XK_BackSpace,	spawn,		{.v = promptreboot} },
	{ MODKEY|ShiftMask,		XK_x,		spawn,		{.v = promptshutdown} },
	{ MODKEY,			XK_r,		spawn,		{.v = filebrowser} },
	{ MODKEY,			XK_i,		spawn,		{.v = systeminfo} },
	{ MODKEY,			XK_e,		spawn,		{.v = email} },
	{ MODKEY,			XK_n,		spawn,		{.v = rssreader} },
	{ MODKEY,			XK_m,		spawn,		{.v = musicplayer} },
	{ MODKEY|ShiftMask,		XK_m,		spawn,		{.v = toggleaudio} },
	{ MODKEY,			XK_p,		spawn,		{.v = pauseaudio} },
	{ MODKEY|ShiftMask,		XK_p,		spawn,		{.v = pauseall} },

	{ MODKEY,                       XK_d,      spawn,          {.v = dmenucmd } },
	{ MODKEY,			XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_c,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_v,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	/* { MODKEY,                       XK_Return, zoom,           {0} }, */
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY,			XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	/* { MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} }, */
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_g,      setlayout,      {.v = &layouts[3]} },
	/* { MODKEY|ControlMask,		XK_comma,  cyclelayout,    {.i = -1 } }, */
	/* { MODKEY|ControlMask,           XK_period, cyclelayout,    {.i = +1 } }, */
	{ MODKEY,			XK_space,  zoom,      {0} },
	{ MODKEY|ShiftMask,           XK_space, cyclelayout,    {.i = +1 } },
	/* { MODKEY,                       XK_space,  setlayout,      {0} }, */
	/* { MODKEY|ShiftMask,             XK_space,  togglefloating, {0} }, */
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	/* { MODKEY,                       XK_comma,  focusmon,       {.i = -1 } }, */
	/* { MODKEY,                       XK_period, focusmon,       {.i = +1 } }, */
	/* { MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } }, */
	/* { MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } }, */
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
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

