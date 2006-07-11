/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <util.h>

static char version[] = "gridsel - " VERSION ", (C)opyright MMVI Anselm R. Garbe\n";

static void
usage()
{
	fprintf(stderr, "%s\n", "usage: gridsel [-v]\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	unsigned char *data;
	unsigned long i, offset, len, remain;

	/* command line args */
	if(argc > 1) {
		if(!strncmp(argv[1], "-v", 3)) {
			fprintf(stdout, "%s", version);
			exit(0);
		} else
			usage();
	}
	len = offset = remain = 0;
	do {
		data = getselection(offset, &len, &remain);
		for(i = 0; i < len; i++)
			putchar(data[i]);
		offset += len;
		free(data);
	}
	while(remain);
	if(offset)
		putchar('\n');
	return 0;
}
