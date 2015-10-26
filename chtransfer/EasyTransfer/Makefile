#
# Makefile
#
# (c) 2003-2011 Thomas Giesel
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#
# Thomas Giesel skoe@directbox.com
#

include ../../Makefile.config

CFLAGS+=-O3 -Wall -W -Wextra

BUILDDIR=../../build-$(TARGET)

here := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
top_dir := $(here)

# don't delete intermediate files
#.SECONDARY:

.SILENT:

.PHONY: world
world: all

CFLAGS+=-I../../chacolib
CXXFLAGS+=-I../../chacolib

prj_name      := chtransfer

cxxflags       = $(CXXFLAGS)
cflags         = $(CFLAGS)
clibs          = $(CLIBS)
cxxlibs        = $(CXXLIBS)
ldflags        = $(LDFLAGS)

ifeq ($(win32), yes)
    app_name      := ChTransfer$(EXE)
    ef3xfer_name  := chxfer$(EXE)
    host          := i686-w64-mingw32
    cxx           := $(host)-c++
    cc            := $(host)-gcc
    windres       := $(host)-windres
    ldflags       += -static-libstdc++ -static-libgcc
#    outbase       := $(BUILDDIR)
    outbase       := obj
    icon_resize   := -resize 32x32
else
    app_name      := ChTransfer$(EXE)
    ef3xfer_name  := chxfer$(EXE)
    cxx           := c++
    cc            := gcc
#    outbase       := $(BUILDDIR)
    outbase       := obj
    cxxflags      += $(shell wx-config --cxxflags)
    cxxlibs       += $(shell wx-config --libs) -lpthread
#    cxxlibs       += $(shell wx-config --libs) -lftdi -lpthread
#    clibs         += -lftd
	clibs         += $(BUILDDIR)/chacolib.a $(CLIBS)
	cxxlibs         += $(BUILDDIR)/chacolib.a $(CLIBS)
    icon_resize   :=
endif

ifneq "$(release)" "yes"
	version        := $(shell date +%y%m%d-%H%M)
	version_suffix :=
else
	version        := 1.3.0
	version_suffix := -$(version)
endif

#outdir        := $(outbase)
outdir        := $(BUILDDIR)
objdir        := $(outbase)
srcdir        := src

cflags    += -DVERSION=\"$(version)\"
cflags    += -I$(objdir)
cxxflags  += -DVERSION=\"$(version)\"
cxxflags  += -I$(objdir)

.PHONY: pre_deps

ifeq ($(win32), yes)
    include make/win32-cross-mingw/cross-wx.mk
#    include make/win32-cross-mingw/cross-ftdi.mk
#pre_deps: libftdi libusb install-wxwidgets
pre_deps: libusb install-wxwidgets
else
pre_deps:
endif


###############################################################################
###############################################################################
# Input and output files
###############################################################################
###############################################################################

# list of source files to be compiled
src := 
src += EasyTransferApp.cpp
src += EasyTransferMainFrame.cpp
src += WorkerThread.cpp
src += TabStartPRG.cpp
src += TabWriteDisk.cpp
src += TabUSBTest.cpp
src += ef3xfer_transport.c
src += ef3xfer_log.c
src += ef3xfer_file.c
src += ef3xfer_d64.c
src += ef3xfer_usb_test.c
src += str_to_key.c
src += chbidir.c
src += easytransfer.png

ef3xfer_src :=
ef3xfer_src += ef3xfer_main.c
ef3xfer_src += ef3xfer_log.c
ef3xfer_src += ef3xfer_transport.c
ef3xfer_src += ef3xfer_file.c
ef3xfer_src += ef3xfer_d64.c
ef3xfer_src += ef3xfer_usb_test.c
ef3xfer_src += str_to_key.c
ef3xfer_src += chbidir.c

# additional object files
obj	:=
obj += $(objdir)/d64writer/d64writer.o
obj += $(objdir)/usbtest/usbtest.o

ef3xfer_obj	:=
ef3xfer_obj += $(objdir)/d64writer/d64writer.o
ef3xfer_obj += $(objdir)/usbtest/usbtest.o

# resource files to be built/copied
res := easytransfer.png

###############################################################################
###############################################################################
# Variable transformations
###############################################################################
###############################################################################

###############################################################################
# Transform all names from $(src)/*.cpp|c|png to out/obj/foo.o or
# out/obj/foo.xpm
#
src_cpp := $(filter %.cpp,$(src))
obj     += $(addprefix $(objdir)/, $(src_cpp:.cpp=.o))
src_c   := $(filter %.c,$(src))
obj     += $(addprefix $(objdir)/, $(src_c:.c=.o))
src_png := $(filter %.png,$(src))
xpm     := $(addprefix $(objdir)/, $(src_png:.png=.xpm))

ef3xfer_obj += $(addprefix $(objdir)/, $(ef3xfer_src:.c=.o))

outres  := $(addprefix $(outdir)/res/, $(res))

# Poor men's dependencies: Let all files depend from all header files
headers := $(wildcard $(srcdir)/*.h) $(xpm)

###############################################################################
###############################################################################
# Rules
###############################################################################
###############################################################################

.PHONY: all

all: $(outdir)/$(app_name) $(outdir)/$(ef3xfer_name) \
	$(outres) $(outdoc) $(dll) $(src_archives)

$(outdir)/$(app_name): $(obj) | $(outdir) pre_deps
	$(cxx) $(ldflags) $(obj) -o $@ $(cxxlibs)

# quick and dirty: command line tool
.PHONY: ef3xfer
ef3xfer: $(outdir)/$(ef3xfer_name)
$(outdir)/$(ef3xfer_name): $(ef3xfer_obj) | $(outdir) pre_deps
	$(cc) $(ldflags) $(cflags) -o $@ $(ef3xfer_obj) $(clibs)

$(outdir) $(objdir):
	mkdir -p $@

$(objdir)/%.o: $(srcdir)/%.cpp $(headers) | $(objdir) pre_deps
	$(cxx) -c $(cxxflags) -o $@ $<

$(objdir)/%.o: $(srcdir)/%.c $(headers) | $(objdir) pre_deps
	$(cc) -c $(cflags) -o $@ $<

$(objdir)/%.o: $(objdir)/%.c $(headers) | pre_deps
	$(cc) -c $(cflags) -o $@ $<

$(objdir)/%.c: $(srcdir)/%.prg | $(objdir) pre_deps
	mkdir -p $(dir $@)
	python make/bin2c.py $< $@ $(notdir $(basename $@))

$(objdir)/%.res.o: $(srcdir)/%.rc | $(objdir) pre_deps
	$(windres) -I $(objdir) $< $@

ifeq ($(TARGET),win)
# n/a
$(objdir)/%.xpm: $(srcdir)/../res/%.xpm | $(objdir) pre_deps
	echo cp $< $@
	cp $< $@
endif

# generate xpm
ifneq ($(TARGET),win)
$(objdir)/%.xpm: $(srcdir)/../res/%.png | $(objdir) pre_deps
	convert $(icon_resize) $< $@.tmp.xpm
	cat $@.tmp.xpm | sed "\
			s/static char/static const char/;\
			s/[\._]tmp//;\
			s/\.xpm/_xpm/" > $@

endif

$(outdir)/%: $(srcdir)/../%
	mkdir -p $(dir $@)
	cp $< $@

$(srcdir)/d64writer/d64writer.prg: always
	$(MAKE) -C $(dir $@)
$(srcdir)/usbtest/usbtest.prg: always
	$(MAKE) -C $(dir $@)

$(objdir)/EasyTransfer.res.o: $(objdir)/easytransfer.ico

# copy .ico
ifeq ($(TARGET),win)
$(objdir)/%.ico: $(srcdir)/../res/%.ico | $(objdir) pre_deps
	cp $< $@
endif

# generate .ico
ifneq ($(TARGET),win)
$(objdir)/%.ico: $(srcdir)/../res/%.png | $(objdir) pre_deps
	convert $< $@
endif

.PHONY: always
always:

###############################################################################
# make clean the simple way
#
.PHONY: clean
clean:
#	rm -rf $(outbase)
	$(MAKE) -C src/d64writer clean
	$(MAKE) -C src/usbtest clean
	rm -f  $(obj) $(ef3xfer_obj)
	rm -f $(objdir)/easytransfer.xpm.tmp.xpm
	rm -rf $(objdir)/d64writer/
	rm -rf $(objdir)/usbtest/
	rm -rf $(objdir)

