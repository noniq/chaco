
include Makefile.config

.PHONY: all chacolib chacocmd wxchaco updater

all: chacolib chacocmd wxchaco updater

chacolib:
	$(MAKE) -C chacolib all

chacocmd:
	$(MAKE) -C chacocmd all

wxchaco:
	$(MAKE) -C wxchaco all

updater:
	$(MAKE) -C updater all
