
include Makefile.config

.PHONY: all chacolib updater chacocmd wxchaco

all: chacolib chacocmd wxchaco updater

chacolib:
	$(MAKE) -C chacolib all

updater:
	$(MAKE) -C updater all

chacocmd:
	$(MAKE) -C chacocmd all

wxchaco:
	$(MAKE) -C wxchaco all

