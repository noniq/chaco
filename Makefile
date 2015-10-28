
.SILENT:

include Makefile.config

.PHONY: all chacolib updater chacocmd chcodenet chshot chusb chmon chtransfer wxchaco

all: chacolib chacocmd updater chcodenet chshot chusb chmon chtransfer wxchaco

chacolib:
	$(MAKE) -C chacolib all

chacocmd:
	$(MAKE) -C chacocmd all

updater:
	$(MAKE) -C updater all

chcodenet:
	$(MAKE) -C chcodenet all

chshot:
	$(MAKE) -C chshot all

chusb:
	$(MAKE) -C chusb all

chmon:
	$(MAKE) -C chmon all

chtransfer:
	$(MAKE) -C chtransfer all

wxchaco:
	$(MAKE) -C wxchaco all

clean:
	$(MAKE) -C chacolib clean
	$(MAKE) -C chacocmd clean
	$(MAKE) -C updater clean
	$(MAKE) -C chcodenet clean
	$(MAKE) -C chshot clean
	$(MAKE) -C chusb clean
	$(MAKE) -C chmon clean
	$(MAKE) -C chtransfer clean
	$(MAKE) -C wxchaco clean

