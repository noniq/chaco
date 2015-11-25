
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

$(BUILDDIR)/wxupdater$(EXE): $(SOURCES) $(HEADER) Makefile data.c coreversion.h
#	$(CXX) $(CFLAGS) -o $(BUILDDIR)/wxupdater$(EXE) $(SOURCES) $(BUILDDIR)/chacolib.a $(CLIBS)
	$(CXX) $(CFLAGS) -o $(BUILDDIR)/wxupdater$(EXE) $(SOURCES) ../chacolib/chacolib.c $(CLIBS)
	-$(UPX) $(BUILDDIR)/wxupdater$(EXE)

coreversion.h: update/*.rbf update/*.bin
	echo "#define __COREVERSION__" '"'`ls update/*.rbf | sed 's:.*chameleon_\(.*\).rbf:\1:'`'"' > coreversion.h

data.c: mkdata$(EXE) update/*.rbf update/*.bin
	./mkdata$(EXE) update/*.rbf update/*.bin data.c

mkdata$(EXE): mkdata.c
	$(CC) -o mkdata$(EXE) mkdata.c

clean:
	$(RM) $(BUILDDIR)/wxupdater$(EXE)
	$(RM) ./mkdata$(EXE)

