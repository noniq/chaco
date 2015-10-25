CC65_HOME=~/Desktop/cc65/github/cc65
CA65=$(CC65_HOME)/bin/ca65
CL65=$(CC65_HOME)/bin/cl65
AR65=$(CC65_HOME)/bin/ar65

libname := libef3usb

inc :=

obj := 
obj += obj/ef3usb_check_cmd.o
obj += obj/ef3usb_recieve_data.o
obj += obj/ef3usb_send_data.o
obj += obj/ef3usb_send_str.o
obj += obj/chbidir.o
obj += obj/chbidir-c.o

.PHONY: all
all: $(libname).lib

obj/%.o: src/%.c | obj
	$(CL65) -t c64 -c -o $@ $<
obj/%.o: src/%.s $(inc) | obj
	$(CA65) -I src -t c64 -o $@ $<

obj:
	mkdir -p obj

$(libname).lib: $(obj)
	rm -f $@
	$(AR65) a $@ $(obj)

.PHONY: clean
clean:
	rm -f $(obj)
	rm -rf obj
	rm -f $(libname).lib

.PHONY: distclean
distclean: clean
	rm -f *~
