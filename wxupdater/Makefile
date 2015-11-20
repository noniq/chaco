
include ../Makefile.config

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

all: $(BUILDDIR)/wxupdater$(EXE)

$(BUILDDIR)/wxupdater$(EXE): $(SOURCES) $(HEADER) Makefile
#	$(CXX) $(CFLAGS) -o $(BUILDDIR)/wxupdater$(EXE) $(SOURCES) $(BUILDDIR)/chacolib.a $(CLIBS)
	$(CXX) $(CFLAGS) -o $(BUILDDIR)/wxupdater$(EXE) $(SOURCES) ../chacolib/chacolib.c $(CLIBS)

clean:
	$(RM) $(BUILDDIR)/wxupdater$(EXE)
