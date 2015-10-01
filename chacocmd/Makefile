
CFLAGS+=-DLIBUSB
CLIBS+=-lusb-1.0

CFLAGS+=-DPOSIX
CFLAGS+=-DLINUX

#CFLAGS+=-O3 -Wall -W -Wextra

BUILDDIR=../build-linux

CFLAGS+=-I../chacolib

all: $(BUILDDIR)/chacocmd

$(BUILDDIR)/chacocmd: chacocmd.c Makefile
	$(CC) $(CFLAGS) -o $(BUILDDIR)/chacocmd chacocmd.c $(BUILDDIR)/chacolib.a $(CLIBS)

clean:
	$(RM) $(BUILDDIR)/chacocmd
