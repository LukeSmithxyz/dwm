# gridwm - grid window manager
#   (C)opyright MMVI Anselm R. Garbe

include config.mk

SRC      = wm.c
OBJ      = ${SRC:.c=.o}

all: gridwm
	@echo finished

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: wm.h

gridwm: ${OBJ}
	@echo LD $@
	@${CC} -o $@ ${OBJ} ${X11LDFLAGS}

clean:
	rm -f gridwm *.o
