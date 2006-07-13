/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "wm.h"

void
error(const char *errstr, ...) {
	va_list ap;
	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(1);
}

static void
bad_malloc(unsigned int size)
{
	fprintf(stderr, "fatal: could not malloc() %d bytes\n",
			(int) size);
	exit(1);
}

void *
emallocz(unsigned int size)
{
	void *res = calloc(1, size);
	if(!res)
		bad_malloc(size);
	return res;
}

void *
emalloc(unsigned int size)
{
	void *res = malloc(size);
	if(!res)
		bad_malloc(size);
	return res;
}

void *
erealloc(void *ptr, unsigned int size)
{
	void *res = realloc(ptr, size);
	if(!res)
		bad_malloc(size);
	return res;
}

char *
estrdup(const char *str)
{
	char *res = strdup(str);
	if(!res)
		bad_malloc(strlen(str));
	return res;
}

void
swap(void **p1, void **p2)
{
	void *tmp = *p1;
	*p1 = *p2;
	*p2 = tmp;
}

void
spawn(char *argv[])
{
	if(!argv || !argv[0])
		return;
	if(fork() == 0) {
		if(fork() == 0) {
			if(dpy)
				close(ConnectionNumber(dpy));
			setsid();
			execvp(argv[0], argv);
			fprintf(stderr, "dwm: execvp %s", argv[0]);
			perror(" failed");
		}
		exit (0);
	}
	wait(0);
}
