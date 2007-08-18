/* See LICENSE file for copyright and license details. */
#include "dwm.h"
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xutil.h>

/* static */

typedef struct {
	const char *prop;
	const char *tags;
	Bool isfloating;
} Rule;

typedef struct {
	regex_t *propregex;
	regex_t *tagregex;
} Regs;

TAGS
RULES

static Regs *regs = NULL;
static unsigned int nrules = 0;
static char prop[512];

static unsigned int
idxoftag(const char *tag) {
	unsigned int i;

	for(i = 0; i < ntags; i++)
		if(tags[i] == tag)
			return i;
	return 0;
}

/* extern */

void
applyrules(Client *c) {
	unsigned int i, j;
	regmatch_t tmp;
	Bool matched = False;
	XClassHint ch = { 0 };

	/* rule matching */
	XGetClassHint(dpy, c->win, &ch);
	snprintf(prop, sizeof prop, "%s:%s:%s",
			ch.res_class ? ch.res_class : "",
			ch.res_name ? ch.res_name : "", c->name);
	for(i = 0; i < nrules; i++)
		if(regs[i].propregex && !regexec(regs[i].propregex, prop, 1, &tmp, 0)) {
			c->isfloating = rules[i].isfloating;
			for(j = 0; regs[i].tagregex && j < ntags; j++) {
				if(!regexec(regs[i].tagregex, tags[j], 1, &tmp, 0)) {
					matched = True;
					c->tags[j] = True;
				}
			}
		}
	if(ch.res_class)
		XFree(ch.res_class);
	if(ch.res_name)
		XFree(ch.res_name);
	if(!matched)
		for(i = 0; i < ntags; i++)
			c->tags[i] = seltags[i];
}

void
compileregs(void) {
	unsigned int i;
	regex_t *reg;

	if(regs)
		return;
	nrules = sizeof rules / sizeof rules[0];
	regs = emallocz(nrules * sizeof(Regs));
	for(i = 0; i < nrules; i++) {
		if(rules[i].prop) {
			reg = emallocz(sizeof(regex_t));
			if(regcomp(reg, rules[i].prop, REG_EXTENDED))
				free(reg);
			else
				regs[i].propregex = reg;
		}
		if(rules[i].tags) {
			reg = emallocz(sizeof(regex_t));
			if(regcomp(reg, rules[i].tags, REG_EXTENDED))
				free(reg);
			else
				regs[i].tagregex = reg;
		}
	}
}

Bool
isvisible(Client *c) {
	unsigned int i;

	for(i = 0; i < ntags; i++)
		if(c->tags[i] && seltags[i])
			return True;
	return False;
}

void
tag(const char *arg) {
	unsigned int i;

	if(!sel)
		return;
	for(i = 0; i < ntags; i++)
		sel->tags[i] = arg == NULL;
	i = idxoftag(arg);
	if(i >= 0 && i < ntags)
		sel->tags[i] = True;
	saveprops(sel);
	arrange();
}

void
togglefloating(const char *arg) {
	if(!sel || isfloating())
		return;
	sel->isfloating = !sel->isfloating;
	if(sel->isfloating) {
		resize(sel, sel->x, sel->y, sel->w, sel->h, True);
		saveprops(sel);
	}
	arrange();
}

void
toggletag(const char *arg) {
	unsigned int i, j;

	if(!sel)
		return;
	i = idxoftag(arg);
	sel->tags[i] = !sel->tags[i];
	for(j = 0; j < ntags && !sel->tags[j]; j++);
	if(j == ntags)
		sel->tags[i] = True;
	saveprops(sel);
	arrange();
}

void
toggleview(const char *arg) {
	unsigned int i, j;

	i = idxoftag(arg);
	seltags[i] = !seltags[i];
	for(j = 0; j < ntags && !seltags[j]; j++);
	if(j == ntags)
		seltags[i] = True; /* cannot toggle last view */
	savedwmprops();
	arrange();
}

void
view(const char *arg) {
	unsigned int i;

	for(i = 0; i < ntags; i++)
		seltags[i] = arg == NULL;
	i = idxoftag(arg);
	if(i >= 0 && i < ntags)
		seltags[i] = True;
	savedwmprops();
	arrange();
}
