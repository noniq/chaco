include ../Makefile.config

CFLAGS+=-O3 -Wall -W -Wextra

BUILDDIR=../build-$(TARGET)

CFLAGS+=-I../chacolib

ifeq ($(TARGET),linux)
#CLIBS+=/usr/lib/libreadline.so
CLIBS+=-lreadline
endif

all:	$(BUILDDIR)/chmon.prg $(BUILDDIR)/chmon$(EXE)

$(BUILDDIR)/chmon.prg: $(BUILDDIR)/chmon.o
	$(CC65PREFIX)ld65 -o $(BUILDDIR)/chmon.prg -t none $(BUILDDIR)/chmon.o --start-addr 0xbffe

$(BUILDDIR)/chmon.o: chserv.s
	$(CC65PREFIX)ca65 -t c64 -o $(BUILDDIR)/chmon.o chserv.s

#chmon.prg: chserv.s
#	$(CC65PREFIX)cl65 -o chmon.prg -t c64 chserv.s -t none --start-addres 0xc7fe


$(BUILDDIR)/chmon$(EXE): chmon.c Makefile
	$(CC) $(CFLAGS) -o $(BUILDDIR)/chmon$(EXE) chmon.c $(BUILDDIR)/chacolib.a $(CLIBS)

clean:
	$(RM) $(BUILDDIR)/chmon$(EXE)
	$(RM) $(BUILDDIR)/chmon.o
	$(RM) $(BUILDDIR)/chmon.prg

send: $(BUILDDIR)/chmon$(EXE) $(BUILDDIR)/chmon.prg
	$(BUILDDIR)/chmon$(EXE) --send $(BUILDDIR)/chmon.prg
