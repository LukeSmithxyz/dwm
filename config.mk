# Customize to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

VERSION = 0.4

# includes and libs
LIBS = -L${PREFIX}/lib -L/usr/lib -lc -L${X11LIB} -lX11

# Linux/BSD
CFLAGS = -Os -I. -I${PREFIX}/include -I/usr/include -I${X11INC} \
	-DVERSION=\"${VERSION}\"
LDFLAGS = ${LIBS}
#CFLAGS = -g -Wall -O2 -I. -I${PREFIX}/include -I/usr/include -I${X11INC} \
#	-DVERSION=\"${VERSION}\"
#LDFLAGS = -g ${LIBS}


# Solaris
#CFLAGS = -fast -xtarget=ultra ${INCLUDES} -DVERSION=\"${VERSION}\"
#LIBS += -lnsl -lsocket

AR = ar cr
CC = cc
RANLIB = ranlib
