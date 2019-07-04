/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx  = 3;        /* border pixel of windows */
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

static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

/** Function to shift the current view to the left/right
 *
 * @param: "arg->i" stores the number of tags to shift right (positive value)
 *          or left (negative value)
 */
void
shiftview(const Arg *arg) {
	Arg shifted;

	if(arg->i > 0) // left circular shift
		shifted.ui = (selmon->tagset[selmon->seltags] << arg->i)
		   | (selmon->tagset[selmon->seltags] >> (LENGTH(tags) - arg->i));

	else // right circular shift
		shifted.ui = selmon->tagset[selmon->seltags] >> (- arg->i)
		   | selmon->tagset[selmon->seltags] << (LENGTH(tags) + arg->i);

	view(&shifted);
}

/* tagging */

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
#include "fibonacci.c"
static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",	tile },    /* first entry is default */
	{ "[M]",	monocle },
 	{ "[@]",	spiral },
	{ "|M|",	centeredmaster },
	{ "><>",	NULL },    /* no layout function means floating behavior */
	{ ">M>",	centeredfloatingmaster },
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

#include "movestack.c"
static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,			24,		killclient,	{0} },
	{ MODKEY,                       44,		focusstack,	{.i = +1 } },
	{ MODKEY,                       45,		focusstack,	{.i = -1 } },
	{ MODKEY|ShiftMask,             44,		movestack,	{.i = +1 } },
	{ MODKEY|ShiftMask,             45,		movestack,	{.i = -1 } },
	{ MODKEY,			65,	zoom,		{0} },
	{ MODKEY,                       43,		setmfact,	{.f = -0.05} },
	{ MODKEY,                       46,		setmfact,	{.f = +0.05} },
	{ MODKEY|ShiftMask,		31,		incnmaster,	{.i = +1 } },
	{ MODKEY|ShiftMask,		32,		incnmaster,	{.i = -1 } },
	{ MODKEY,			56,		togglebar,	{0} },
	{ MODKEY,			28,		setlayout,	{.v = &layouts[0]} },
	{ MODKEY,			41,		setlayout,	{.v = &layouts[1]} },
	{ MODKEY,			29,		setlayout,	{.v = &layouts[2]} },
	{ MODKEY,			30,		setlayout,	{.v = &layouts[3]} },
	{ MODKEY,			31,		setlayout,	{.v = &layouts[4]} },
	{ MODKEY,			32,		setlayout,	{.v = &layouts[5]} },
	{ MODKEY,			23,		view,		{0} },
	{ MODKEY,			54,	view,		{0} },
	{ MODKEY|ShiftMask,		65,	togglefloating,	{0} },
	/* { MODKEY,                       XK_space,  setlayout,      {0} }, */
	/* { MODKEY,                       XK_comma,  focusmon,       {.i = -1 } }, */
	/* { MODKEY,                       XK_period, focusmon,       {.i = +1 } }, */
	/* { MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } }, */
	/* { MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } }, */
	TAGKEYS(			10,			0)  //1
	TAGKEYS(			11,			1)  //2
	TAGKEYS(			12,			2)  //3
	TAGKEYS(			13,			3)  //4
	TAGKEYS(			14,			4)  //5
	TAGKEYS(			15,			5)  //6
	TAGKEYS(			16,			6)  //7
	TAGKEYS(			17,			7)  //8
	TAGKEYS(			18,			8)  //9
	{ MODKEY,                       19,		view,		{.ui = ~0 } },
	{ MODKEY|ShiftMask,		19,		tag,		{.ui = ~0 } },
	{ MODKEY,			68,		quit,		{0} },
	{ MODKEY,			42,		shiftview,	{ .i = -1 } },
	{ MODKEY,			47,	shiftview,	{ .i = 1 } },
	{ MODKEY,			112,	shiftview,	{ .i = -1 } },
	{ MODKEY,			117,	shiftview,	{ .i = 1 } },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	/* { ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } }, */
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

