
include ../Makefile.config

CFLAGS+=-O3 -Wall -W -Wextra

BUILDDIR=../build-$(TARGET)

CFLAGS+=-I../chacolib

all: $(BUILDDIR)/chacocmd$(EXE)

$(BUILDDIR)/chacocmd$(EXE): chacocmd.c Makefile
	$(CC) $(CFLAGS) -o $(BUILDDIR)/chacocmd$(EXE) chacocmd.c $(BUILDDIR)/chacolib.a $(CLIBS)

clean:
	$(RM) $(BUILDDIR)/chacocmd$(EXE)
