# gridwm - grid window manager
#   (C)opyright MMVI Anselm R. Garbe

include config.mk

WMSRC = wm.c draw.c util.c
WMOBJ = ${WMSRC:.c=.o}
MENSRC = menu.c draw.c util.c
MENOBJ = ${MENSRC:.c=.o}
MAN = gridwm.1
BIN = gridwm gridmenu     

all: config gridwm gridmenu
	@echo finished

config:
	@echo gridwm build options:
	@echo "LIBS     = ${LIBS}"
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${WMOBJ}: wm.h draw.h config.h util.h

gridmenu: ${MENOBJ}
	@echo LD $@
	@${CC} -o $@ ${MENOBJ} ${LDFLAGS}

gridwm: ${WMOBJ}
	@echo LD $@
	@${CC} -o $@ ${WMOBJ} ${LDFLAGS}

clean:
	rm -f gridwm gridmenu *.o core

dist: clean
	mkdir -p gridwm-${VERSION}
	cp -R Makefile README LICENSE config.mk *.h *.c ${MAN} gridwm-${VERSION}
	tar -cf gridwm-${VERSION}.tar gridwm-${VERSION}
	gzip gridwm-${VERSION}.tar
	rm -rf gridwm-${VERSION}

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
