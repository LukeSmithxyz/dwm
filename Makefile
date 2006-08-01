# dwm - dynamic window manager
#   (C)opyright MMVI Anselm R. Garbe

include config.mk

SRC = client.c draw.c event.c main.c tag.c util.c
OBJ = ${SRC:.c=.o}
MAN1 = dwm.1 
BIN = dwm

all: options dwm
	@echo finished

options:
	@echo dwm build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: dwm.h

dwm: ${OBJ}
	@echo LD $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f dwm *.o dwm-${VERSION}.tar.gz

dist: clean
	mkdir -p dwm-${VERSION}
	cp -R Makefile README LICENSE config.mk *.h *.c ${MAN1} dwm-${VERSION}
	tar -cf dwm-${VERSION}.tar dwm-${VERSION}
	gzip dwm-${VERSION}.tar
	rm -rf dwm-${VERSION}

install: all
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f ${BIN} ${DESTDIR}${PREFIX}/bin
	@for i in ${BIN}; do \
		chmod 755 ${DESTDIR}${PREFIX}/bin/`basename $$i`; \
	done
	@echo installed executable files to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@cp -f ${MAN1} ${DESTDIR}${MANPREFIX}/man1
	@for i in ${MAN1}; do \
		chmod 444 ${DESTDIR}${MANPREFIX}/man1/`basename $$i`; \
	done
	@echo installed manual pages to ${DESTDIR}${MANPREFIX}/man1

uninstall:
	for i in ${BIN}; do \
		rm -f ${DESTDIR}${PREFIX}/bin/`basename $$i`; \
	done
	for i in ${MAN1}; do \
		rm -f ${DESTDIR}${MANPREFIX}/man1/`basename $$i`; \
	done
