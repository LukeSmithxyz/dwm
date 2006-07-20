/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "dwm.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/* static */

static void
bad_malloc(unsigned int size)
{
	fprintf(stderr, "fatal: could not malloc() %d bytes\n",
			(int) size);
	exit(EXIT_FAILURE);
}

/* extern */

void *
emallocz(unsigned int size)
{
	void *res = calloc(1, size);

	if(!res)
		bad_malloc(size);
	return res;
}

void
eprint(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void
spawn(Arg *arg)
{
	char **argv = (char **)arg->argv;

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
		exit(EXIT_FAILURE);
	}
	wait(0);
}
