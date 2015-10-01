
CFLAGS+=-DLIBUSB
CLIBS+=-lusb-1.0

CFLAGS+=-DPOSIX

#CFLAGS+=-O3 -Wall -W -Wextra

BUILDDIR=../build-linux

CFLAGS+=-I../chacolib

all: $(BUILDDIR)/update

$(BUILDDIR)/update: update.c Makefile
	$(CC) $(CFLAGS) -o $(BUILDDIR)/update update.c $(BUILDDIR)/chacolib.a $(CLIBS)

clean:
	$(RM) $(BUILDDIR)/update
