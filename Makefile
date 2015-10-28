
.SILENT:

include Makefile.config

CMDTOOLS=chacolib updater chacocmd chcodenet chshot chusb chmon chxfer
WXTOOLS=ChTransfer Chaco
ALLTOOLS=$(CMDTOOLS)
ifneq ($(TARGET),osx)
ALLTOOLS+=$(WXTOOLS)
endif

.PHONY: all zip $(ALLTOOLS)

all: $(ALLTOOLS) zip

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

chxfer:
	$(MAKE) -C chtransfer chxfer

ifneq ($(TARGET),osx)
ChTransfer:
	$(MAKE) -C chtransfer all

Chaco:
	$(MAKE) -C wxchaco all
endif

zip: $(ALLTOOLS)
	zip -q chameleon-tools-$(TARGET)-`date +"%Y%m%d"`.zip \
		license.txt \
		readme.txt
	cd ./build-$(TARGET); zip -q ../chameleon-tools-$(TARGET)-`date +"%Y%m%d"`.zip \
		chacocmd$(EXE) \
		updater$(EXE) \
		chcodenet$(EXE) \
		chshot$(EXE) \
		chusb$(EXE) \
		chusb.prg \
		chmon$(EXE) \
		chmon.prg

ifneq ($(TARGET),osx)
	cd ./build-$(TARGET); zip -q ../chameleon-tools-$(TARGET)-`date +"%Y%m%d"`.zip \
		ChTransfer$(EXE) \
		Chaco$(EXE)
endif
	cp chameleon-tools-$(TARGET)-`date +"%Y%m%d"`.zip chameleon-tools-$(TARGET).zip

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

