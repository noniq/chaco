
include ../Makefile.config

#CFLAGS+=`wx-config --cflags` -D__GXX_ABI_VERSION=1010
#CLIBS+=`wx-config --libs`
#CFLAGS+=`wx-config --cflags --toolkit=gtk2`
#CLIBS+=`wx-config --libs --toolkit=gtk2`
CFLAGS+=`wx-config --cflags`
CLIBS+=`wx-config --libs`

BUILDDIR=../build-$(TARGET)

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

all: $(BUILDDIR)/Chaco$(EXE)

$(BUILDDIR)/Chaco$(EXE): $(SOURCES) $(HEADER) Makefile
#	$(CXX) $(CFLAGS) -o $(BUILDDIR)/Chaco$(EXE) $(SOURCES) $(BUILDDIR)/chacolib.a $(CLIBS)
	$(CXX) $(CFLAGS) -o $(BUILDDIR)/Chaco$(EXE) $(SOURCES) ../chacolib/chacolib.c $(CLIBS)

clean:
	$(RM) $(BUILDDIR)/Chaco$(EXE)
