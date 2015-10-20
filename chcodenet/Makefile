
include ../Makefile.config

CFLAGS+=-O3 -Wall -W -Wextra

BUILDDIR=../build-$(TARGET)

CFLAGS+=-I../chacolib

all: $(BUILDDIR)/chcodenet$(EXE)

$(BUILDDIR)/chcodenet$(EXE): chcodenet.c Makefile
	$(CC) $(CFLAGS) -o $(BUILDDIR)/chcodenet$(EXE) chcodenet.c $(BUILDDIR)/chacolib.a $(CLIBS)

clean:
	$(RM) $(BUILDDIR)/chcodenet$(EXE)
