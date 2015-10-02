
include ../Makefile.config

BUILDDIR=../build-$(TARGET)

all: $(BUILDDIR)/chacolib.a

$(BUILDDIR)/chacolib.a: $(BUILDDIR)/chacolib.o Makefile
	$(AR) cr $(BUILDDIR)/chacolib.a $(BUILDDIR)/chacolib.o

$(BUILDDIR)/chacolib.o: chacolib.c chacolib.h chacolib-libusb.c chacolib-hidapi.c
	$(CC) $(CFLAGS) -c -o $(BUILDDIR)/chacolib.o chacolib.c

clean:
	$(RM) $(BUILDDIR)/chacolib.a
	$(RM) $(BUILDDIR)/chacolib.o
