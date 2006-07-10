/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

extern void error(char *errstr, ...);
extern void *emallocz(unsigned int size);
extern void *emalloc(unsigned int size);
extern void *erealloc(void *ptr, unsigned int size);
extern char *estrdup(const char *str);
#define eassert(a) do { \
		if(!(a)) \
			failed_assert(#a, __FILE__, __LINE__); \
	} while (0)
void failed_assert(char *a, char *file, int line);
void swap(void **p1, void **p2);
