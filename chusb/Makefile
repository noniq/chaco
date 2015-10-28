
include ../Makefile.config

CFLAGS+=-O3 -Wall -W -Wextra

BUILDDIR=../build-$(TARGET)

CFLAGS+=-I../chacolib

all: $(BUILDDIR)/chusb$(EXE) $(BUILDDIR)/chusb.prg

$(BUILDDIR)/chusb.prg: chusb.tas fb.tas format2_runtime.i chbidir.tas
	64tass -q chusb.tas -o $(BUILDDIR)/chusb.prg -a

$(BUILDDIR)/chusb$(EXE): chusb.c chbidir.c Makefile
	$(CC) $(CFLAGS) -o $(BUILDDIR)/chusb$(EXE) chusb.c $(BUILDDIR)/chacolib.a $(CLIBS)

clean:
	$(RM) $(BUILDDIR)/chusb$(EXE)
	$(RM) $(BUILDDIR)/chusb.prg
