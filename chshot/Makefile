
include ../Makefile.config

CFLAGS+=-O3 -Wall -W -Wextra

BUILDDIR=../build-$(TARGET)

CFLAGS+=-I../chacolib

all: $(BUILDDIR)/chshot$(EXE)

$(BUILDDIR)/chshot$(EXE): chshot.c Makefile
	$(CC) $(CFLAGS) -o $(BUILDDIR)/chshot$(EXE) chshot.c $(BUILDDIR)/chacolib.a $(CLIBS)

clean:
	$(RM) $(BUILDDIR)/chshot$(EXE)
