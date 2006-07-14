# dwm - dynamic window manager
#   (C)opyright MMVI Anselm R. Garbe

include config.mk

SRC = bar.c client.c dev.c draw.c event.c main.c util.c
OBJ = ${SRC:.c=.o}
MAN1 = dwm.1 
BIN = dwm

all: config dwm
	@echo finished

config:
	@echo dwm build options:
	@echo "LIBS     = ${LIBS}"
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
	rm -f dwm *.o core

dist: clean
	mkdir -p dwm-${VERSION}
	cp -R Makefile README LICENSE config.mk *.h *.c ${MAN} dwm-${VERSION}
	tar -cf dwm-${VERSION}.tar dwm-${VERSION}
	gzip dwm-${VERSION}.tar
	rm -rf dwm-${VERSION}

install: all
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f ${BIN} ${DESTDIR}${PREFIX}/bin
	@echo installed executable files to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@cp -f ${MAN1} ${DESTDIR}${MANPREFIX}/man1
	@echo installed manual pages to ${DESTDIR}${MANPREFIX}/man1

uninstall:
	for i in ${BIN}; do \
		rm -f ${DESTDIR}${PREFIX}/bin/`basename $$i`; \
	done
	for i in ${MAN1}; do \
		rm -f ${DESTDIR}${MANPREFIX}/man1/`basename $$i`; \
	done
