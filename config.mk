# dwm version
VERSION = 0.6

# Customize below to fit your system

# configheader
CONFIG = config.h

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# includes and libs
INCS = -I/usr/lib -I${X11INC}
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11

# flags
CFLAGS = -O3 ${INCS} -DVERSION=\"${VERSION}\" -DCONFIG=\"${CONFIG}\"
LDFLAGS = ${LIBS}
#CFLAGS = -g -Wall -O2 ${INCS} -DVERSION=\"${VERSION}\" -DCONFIG=\"${CONFIG}\"
#LDFLAGS = -g ${LIBS}

# compiler
CC = cc
