
CFLAGS+=-DLIBUSB
CLIBS+=-lusb-1.0

CFLAGS+=-DPOSIX
CFLAGS+=-DLINUX

CFLAGS+=`wx-config --cflags`
CLIBS+=`wx-config --libs`

#CFLAGS+=-O3 -Wall -W -Wextra

BUILDDIR=../build-linux

CFLAGS+=-I../chacolib

SOURCES=\
	BasicFile.cpp \
	Chameleon.cpp \
	ChFrm.cpp \
	Dbg.cpp \
	main.cpp \
	TaskMgr.cpp \
	Threading.cpp

HEADER=\
	BasicFile.h \
	Chaco.h \
	Chameleon.h \
	ChFrm.h \
	Dbg.h \
	Task.h \
	TaskMgr.h \
	Threading.h

all: $(BUILDDIR)/Chaco

$(BUILDDIR)/Chaco: $(SOURCES) $(HEADER) Makefile
#	$(CXX) $(CFLAGS) -o $(BUILDDIR)/Chaco $(SOURCES) $(BUILDDIR)/chacolib.a $(CLIBS)
	$(CXX) $(CFLAGS) -o $(BUILDDIR)/Chaco $(SOURCES) ../chacolib/chacolib.c $(CLIBS)

clean:
	$(RM) $(BUILDDIR)/Chaco
