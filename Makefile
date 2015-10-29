
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
	rm -f chameleon-tools-$(TARGET)-`date +"%Y%m%d"`.zip
	rm -f chameleon-tools-$(TARGET).zip

	rm -f chameleon-tools-source-`date +"%Y%m%d"`.zip
	rm -f chameleon-tools-source.zip

	zip -qr chameleon-tools-source-`date +"%Y%m%d"`.zip \
		chacolib \
		chacocmd \
		updater \
		wxchaco \
		chcodenet \
		chmon \
		chshot \
		chtransfer \
		chusb \
		doc \
		Makefile \
		Makefile.config \
		readme.txt \
		license.txt \
		-x chtransfer/EasyTransfer/obj/* \
		-x chtransfer/EasyTransfer/obj \
		-x chtransfer/EasyTransfer/obj/d64writer/* \
		-x chtransfer/EasyTransfer/obj/d64writer \
		-x chtransfer/EasyTransfer/src/d64writer/obj/* \
		-x chtransfer/EasyTransfer/src/d64writer/obj \
		-x chtransfer/EasyTransfer/src/d64writer/d64writer.prg \
		-x chtransfer/EasyTransfer/obj/usbtest/* \
		-x chtransfer/EasyTransfer/obj/usbtest \
		-x chtransfer/EasyTransfer/src/usbtest/obj/* \
		-x chtransfer/EasyTransfer/src/usbtest/obj \
		-x chtransfer/EasyTransfer/src/usbtest/usbtest.prg \
		-x chtransfer/libs/eload/eload.lib \
		-x chtransfer/libs/eload/obj/* \
		-x chtransfer/libs/eload/obj \
		-x chtransfer/libs/libef3usb/libef3usb.lib \
		-x chtransfer/libs/libef3usb/obj/* \
		-x chtransfer/libs/libef3usb/obj \
		-x .DS_Store


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
	mkdir -p ./build-$(TARGET)/doc
	cp chshot/readme.txt ./build-$(TARGET)/doc/readme-chshot.txt
	cp chcodenet/readme.txt ./build-$(TARGET)/doc/readme-chcodenet.txt
	cp chusb/readme.txt ./build-$(TARGET)/doc/readme-chusb.txt
	cp chmon/readme.txt ./build-$(TARGET)/doc/readme-chmon.txt
	cp chtransfer/readme.txt ./build-$(TARGET)/doc/readme-chtransfer.txt
	cd ./build-$(TARGET); zip -mq ../chameleon-tools-$(TARGET)-`date +"%Y%m%d"`.zip \
		doc/readme-chcodenet.txt \
		doc/readme-chshot.txt \
		doc/readme-chusb.txt \
		doc/readme-chmon.txt

ifneq ($(TARGET),osx)
	cd ./build-$(TARGET); zip -q ../chameleon-tools-$(TARGET)-`date +"%Y%m%d"`.zip \
		ChTransfer$(EXE) \
		Chaco$(EXE)
	cd ./build-$(TARGET); zip -mq ../chameleon-tools-$(TARGET)-`date +"%Y%m%d"`.zip \
		doc/readme-chtransfer.txt
endif
	cp chameleon-tools-$(TARGET)-`date +"%Y%m%d"`.zip chameleon-tools-$(TARGET).zip
	cp chameleon-tools-source-`date +"%Y%m%d"`.zip chameleon-tools-source.zip

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

