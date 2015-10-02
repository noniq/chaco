
include ../Makefile.config

BUILDDIR=../build-$(TARGET)

CFLAGS+=-I../chacolib

all: $(BUILDDIR)/update$(EXE)

$(BUILDDIR)/update$(EXE): update.c Makefile
	$(CC) $(CFLAGS) -o $(BUILDDIR)/update$(EXE) update.c $(BUILDDIR)/chacolib.a $(CLIBS)

clean:
	$(RM) $(BUILDDIR)/update$(EXE)
