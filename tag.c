/* (C)opyright MMVI-MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <X11/Xutil.h>


typedef struct {
	const char *clpattern;
	const char *tpattern;
	Bool isfloat;
} Rule;

typedef struct {
	regex_t *clregex;
	regex_t *tregex;
} RReg;

/* static */

TAGS
RULES

static RReg *rreg = NULL;
static unsigned int len = 0;

/* extern */

Client *
getnext(Client *c) {
	for(; c && !isvisible(c); c = c->next);
	return c;
}

Client *
getprev(Client *c) {
	for(; c && !isvisible(c); c = c->prev);
	return c;
}

void
initrregs(void) {
	unsigned int i;
	regex_t *reg;

	if(rreg)
		return;
	len = sizeof rule / sizeof rule[0];
	rreg = emallocz(len * sizeof(RReg));
	for(i = 0; i < len; i++) {
		if(rule[i].clpattern) {
			reg = emallocz(sizeof(regex_t));
			if(regcomp(reg, rule[i].clpattern, REG_EXTENDED))
				free(reg);
			else
				rreg[i].clregex = reg;
		}
		if(rule[i].tpattern) {
			reg = emallocz(sizeof(regex_t));
			if(regcomp(reg, rule[i].tpattern, REG_EXTENDED))
				free(reg);
			else
				rreg[i].tregex = reg;
		}
	}
}

void
settags(Client *c, Client *trans) {
	char prop[512];
	unsigned int i, j;
	regmatch_t tmp;
	Bool matched = trans != NULL;
	XClassHint ch;

	if(matched) {
		for(i = 0; i < ntags; i++)
			c->tags[i] = trans->tags[i];
	}
	else if(XGetClassHint(dpy, c->win, &ch)) {
		snprintf(prop, sizeof prop, "%s:%s:%s",
				ch.res_class ? ch.res_class : "",
				ch.res_name ? ch.res_name : "", c->name);
		for(i = 0; i < len; i++)
			if(rreg[i].clregex && !regexec(rreg[i].clregex, prop, 1, &tmp, 0)) {
				c->isfloat = rule[i].isfloat;
				for(j = 0; rreg[i].tregex && j < ntags; j++) {
					if(!regexec(rreg[i].tregex, tags[j], 1, &tmp, 0)) {
						matched = True;
						c->tags[j] = True;
					}
				}
			}
		if(ch.res_class)
			XFree(ch.res_class);
		if(ch.res_name)
			XFree(ch.res_name);
	}
	if(!matched)
		for(i = 0; i < ntags; i++)
			c->tags[i] = seltag[i];
}

void
tag(Arg *arg) {
	unsigned int i;

	if(!sel)
		return;
	for(i = 0; i < ntags; i++)
		sel->tags[i] = (arg->i == -1) ? True : False;
	if(arg->i >= 0 && arg->i < ntags)
		sel->tags[arg->i] = True;
	arrange();
}

void
toggletag(Arg *arg) {
	unsigned int i;

	if(!sel)
		return;
	sel->tags[arg->i] = !sel->tags[arg->i];
	for(i = 0; i < ntags && !sel->tags[i]; i++);
	if(i == ntags)
		sel->tags[arg->i] = True;
	arrange();
}
