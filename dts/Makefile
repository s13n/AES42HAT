# Change the install directory according to your needs.
INSTALLDIR = /boot/firmware/current/overlays
DTBODIR = .
DTSO = $(wildcard *.dtso)
DTBO = $(DTSO:.dtso=.dtbo)

all: $(DTBO)

%.dtbo: %.dtso
	dtc -I dts -O dtb -o $@ -b 0 -@ $<

clean:
	rm *.dtbo

install: all
	cp *.dtbo $(INSTALLDIR)

.PHONY: all clean install
