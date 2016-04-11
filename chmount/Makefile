
include ../Makefile.config

CFLAGS+=-O3 -Wall -W -Wextra

BUILDDIR=../build-$(TARGET)

CFLAGS+=-I../chacolib
CFLAGS+=-I../include

all: $(BUILDDIR)/chmount$(EXE)

$(BUILDDIR)/chmount$(EXE): chmount.c d64togcr.c g64togcr.c Makefile
	$(CC) $(CFLAGS) -o $(BUILDDIR)/chmount$(EXE) chmount.c g64togcr.c $(BUILDDIR)/chacolib.a $(CLIBS)

clean:
	$(RM) $(BUILDDIR)/chmount$(EXE)
