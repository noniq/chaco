
include ../Makefile.config

BUILDDIR=../build-$(TARGET)

CFLAGS+=-I../chacolib

all: $(BUILDDIR)/flasher$(EXE)

$(BUILDDIR)/flasher$(EXE): update.c Makefile $(BUILDDIR)/chacolib.a
	$(CC) $(CFLAGS) -o $(BUILDDIR)/flasher$(EXE) update.c $(BUILDDIR)/chacolib.a $(CLIBS)

clean:
	$(RM) $(BUILDDIR)/flasher$(EXE)
