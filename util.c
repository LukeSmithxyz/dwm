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

#include "util.h"

static char *shell = NULL;

void
error(char *errstr, ...) {
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
	void *res = strdup(str);
	if(!res)
		bad_malloc(strlen(str));
	return res;
}

void
failed_assert(char *a, char *file, int line)
{
	fprintf(stderr, "Assertion \"%s\" failed at %s:%d\n", a, file, line);
	abort();
}

void
swap(void **p1, void **p2)
{
	void *tmp = *p1;
	*p1 = *p2;
	*p2 = tmp;
}

void
spawn(Display *dpy, const char *cmd)
{
	if(!shell && !(shell = getenv("SHELL")))
		shell = "/bin/sh";

	if(!cmd)
		return;
	if(fork() == 0) {
		if(fork() == 0) {
			setsid();
			if(dpy)
				close(ConnectionNumber(dpy));
			execlp(shell, "shell", "-c", cmd, NULL);
			fprintf(stderr, "gridwm: execvp %s", cmd);
			perror(" failed");
		}
		exit (0);
	}
	wait(0);
}

void
pipe_spawn(char *buf, unsigned int len, Display *dpy, const char *cmd)
{
	unsigned int l, n;
	int pfd[2];

	if(!shell && !(shell = getenv("SHELL")))
		shell = "/bin/sh";

	if(!cmd)
		return;

	if(pipe(pfd) == -1) {
		perror("pipe");
		exit(1);
	}

	if(fork() == 0) {
		setsid();
		if(dpy)
			close(ConnectionNumber(dpy));
		dup2(pfd[1], STDOUT_FILENO);
		close(pfd[0]);
		close(pfd[1]);
		execlp(shell, "shell", "-c", cmd, NULL);
		fprintf(stderr, "gridwm: execvp %s", cmd);
		perror(" failed");
	}
	else {
		n = 0;
		close(pfd[1]);
		while(l > n) {
			if((l = read(pfd[0], buf + n, len - n)) < 1)
				break;
			n += l;
		}
		close(pfd[0]);
		buf[n - 1] = 0;
	}
	wait(0);
}
