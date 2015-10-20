
include Makefile.config

.PHONY: all chacolib updater chacocmd chcodenet wxchaco

all: chacolib chacocmd updater chcodenet wxchaco

chacolib:
	$(MAKE) -C chacolib all

chacocmd:
	$(MAKE) -C chacocmd all

updater:
	$(MAKE) -C updater all

chcodenet:
	$(MAKE) -C chcodenet all

wxchaco:
	$(MAKE) -C wxchaco all

clean:
	$(MAKE) -C chacolib clean
	$(MAKE) -C chacocmd clean
	$(MAKE) -C updater clean
	$(MAKE) -C chcodenet clean
	$(MAKE) -C wxchaco clean

