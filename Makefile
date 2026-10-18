BR_ROOT := /home/ataberk/mangopi-mqpro-br
OUTPUT := $(BR_ROOT)/output/mqpro

CROSS_COMPILE := $(OUTPUT)/host/bin/riscv64-buildroot-linux-gnu-
CC := $(CROSS_COMPILE)gcc
SYSROOT := $(OUTPUT)/host/riscv64-buildroot-linux-gnu/sysroot
MOD_CC := $(CROSS_COMPILE)

KDIR ?= $(OUTPUT)/build/linux-custom

PROJECT := $(CURDIR)/v0.1-alpha
SRC := $(PROJECT)/src
API := $(PROJECT)/API
HAL := $(SRC)/HAL
PTHREAD := $(HAL)/pthread
ALLWINNER := $(HAL)/allwinner
MODULE := $(PROJECT)/kernel/module

OVERLAY := $(BR_ROOT)/br-external/board/mqpro/overlay

CFLAGS := -O2 -Wall -fPIC --sysroot=$(SYSROOT)
LDFLAGS := -shared

API_UI := $(OVERLAY)/usr/lib/api/ui
API_NET := $(OVERLAY)/usr/lib/api/net
API_IO := $(OVERLAY)/usr/lib/api/io
API_IPC := $(OVERLAY)/usr/lib/api/ipc
API_MEDIA := $(OVERLAY)/usr/lib/api/media
API_SYSFUNC := $(OVERLAY)/usr/lib/api/sysfunc
API_STAT := $(OVERLAY)/usr/lib/api/stat

HAL_CON := $(OVERLAY)/usr/lib/hal/con
HAL_SCREEN := $(OVERLAY)/usr/lib/hal/screen
HAL_MOUSE := $(OVERLAY)/usr/lib/hal/mouse
HAL_SOUND := $(OVERLAY)/usr/lib/hal/sound
HAL_ETHERNET := $(OVERLAY)/usr/lib/hal/ethernet
HAL_GSM := $(OVERLAY)/usr/lib/hal/ethernet/gsm
HAL_SYSTEM := $(OVERLAY)/usr/lib/hal/system

AUTUMN_LIB := $(OVERLAY)/usr/lib/autumn
BIN := $(OVERLAY)/bin
SBIN := $(OVERLAY)/sbin
TOOLS := $(OVERLAY)/sbin/autumn-tools
USR_BIN := $(OVERLAY)/usr/bin
MODULES := $(OVERLAY)/lib/modules
FONTS := $(OVERLAY)/usr/share/autumncfonts

.PHONY: all prepare session init libs hal api daemons tools sysui modules install br_build clean

all: prepare session init libs hal api daemons tools sysui modules install

prepare:
	mkdir -p $(API_UI) $(API_NET) $(API_IO) $(API_IPC) $(API_MEDIA) $(API_SYSFUNC) $(API_STAT)
	mkdir -p $(HAL_CON) $(HAL_SCREEN) $(HAL_MOUSE) $(HAL_SOUND)
	mkdir -p $(HAL_ETHERNET) $(HAL_GSM) $(HAL_SYSTEM)
	mkdir -p $(AUTUMN_LIB) $(BIN) $(SBIN) $(TOOLS) $(USR_BIN) $(MODULES) $(FONTS)

session:
	$(CC) $(CFLAGS) $(SRC)/loader.c $(SRC)/tabldioctl.c $(SRC)/sigf.c $(SRC)/session.c $(SRC)/inputd.c $(SRC)/avm.c $(API)/libatmchtn.c -o session -lpthread

init:
	$(CC) $(SRC)/sigf.c $(SRC)/init.c -static -o init

asurfd:
	$(CC) $(CFLAGS) $(SRC)/asurfd.c -o asurfd

modemd:
	$(CC) $(CFLAGS) $(SRC)/modemd.c -o modemd

AutumnGenericLowLMgr:
	$(CC) $(CFLAGS) $(SRC)/AutumnGenericLowLMgr.c -o AutumnGenericLowLMgr

sysui:
	$(CC) $(CFLAGS) $(SRC)/loader.c $(SRC)/tabldioctl.c $(SRC)/pscrfont.c $(SRC)/sysui.c -o sysui -lpthread

# --- HAL Kütüphaneleri (Donanım seviyesi bağımsız kalıyor) ---
libconhal.so:
	$(CC) $(CFLAGS) $(LDFLAGS) $(HAL)/console.c -o libconhal.so

libfbdhal.so:
	$(CC) $(CFLAGS) $(LDFLAGS) $(HAL)/fb_dev.c $(HAL)/drm.c -o libfbdhal.so

libmshal.so:
	$(CC) $(CFLAGS) $(LDFLAGS) $(HAL)/input.c -o libmshal.so

libsndhal.so:
	$(CC) $(CFLAGS) $(LDFLAGS) $(HAL)/sound.c -o libsndhal.so

libethal.so:
	$(CC) $(CFLAGS) $(LDFLAGS) $(HAL)/etht.c -o libethal.so

libpwrhal.so:
	$(CC) $(CFLAGS) $(LDFLAGS) $(HAL)/power.c -o libpwrhal.so

libuarthal.so:
	$(CC) $(CFLAGS) $(LDFLAGS) $(HAL)/uart.c -o libuarthal.so

libsimhal.so:
	$(CC) $(CFLAGS) $(LDFLAGS) $(HAL)/modem.c -o libsimhal.so

libhdmihal.so:
	$(CC) $(CFLAGS) $(LDFLAGS) $(HAL)/hdmi.c -o libhdmihal.so

# --- TEK ÇATI ALTINDA BİRLEŞTİRİLEN BÜTÜN API KÜTÜPHANESİ (libfbdev ve libatmvidec dahil) ---
libatmcore.so:
	$(CC) $(CFLAGS) $(LDFLAGS) \
		$(API)/libatmnls.c \
		$(API)/lpngsysc.c \
		$(API)/libfbdev.c \
		$(API)/libmp4sysc.c \
		$(API)/libatmvidec.c \
		$(API)/widget.c \
		$(API)/libatmio.c \
		$(API)/libatmsndec.c \
		$(API)/libatmeth.c \
		$(API)/libatmdial.c \
		$(API)/libatmui.c \
		$(API)/libatmchtn.c \
		$(API)/libatmjson.c \
		$(API)/libpalette.c \
		$(API)/libatmasurf.c \
		$(API)/libatmtls.c \
		$(API)/libaction.c \
		$(API)/libatmtask.c \
		-o libatmcore.so -lfreetype -lpng

libs: libwhike.so

hal: libconhal.so libfbdhal.so libmshal.so libsndhal.so libethal.so libpwrhal.so libuarthal.so libsimhal.so libhdmihal.so

api: libatmcore.so

daemons: asurfd modemd AutumnGenericLowLMgr

tools:
	$(CC) $(CFLAGS) $(SRC)/autumn-kexec.c -o autumn-kexec
	$(CC) $(CFLAGS) $(SRC)/autumn-atxdb.c -o autumn-atxdb
	$(CC) $(CFLAGS) $(SRC)/autumn-sudo.c -o autumn-sudo

modules:
	@set -e; \
	trap 'rm -f "$(MODULE)/Kbuild"' EXIT INT TERM; \
	printf '%s\n' \
		'obj-m += s800modem.o' \
		'obj-m += avinput.o' \
		'obj-m += a_panicsod.o' \
		'obj-m += tabloader.o' \
		'obj-m += coder.o' \
		> "$(MODULE)/Kbuild"; \
	$(MAKE) -C "$(KDIR)" \
		ARCH=riscv \
		CROSS_COMPILE="$(MOD_CC)" \
		M="$(MODULE)" \
		modules

install:
	cp -f init $(SBIN)/
	cp -f session $(BIN)/
	cp -f asurfd $(BIN)/
	cp -f modemd $(BIN)/
	cp -f AutumnGenericLowLMgr $(BIN)/
	cp -f sysui $(USR_BIN)/

	cp -f autumn-kexec $(TOOLS)/
	cp -f autumn-atxdb $(TOOLS)/
	cp -f autumn-sudo $(TOOLS)/

	# Tek core API kütüphanesini ilgili klasörlere dağıtıyoruz
	cp -f libatmcore.so $(API_UI)/
	cp -f libatmcore.so $(API_NET)/
	cp -f libatmcore.so $(API_IO)/
	cp -f libatmcore.so $(API_IPC)/
	cp -f libatmcore.so $(API_MEDIA)/
	cp -f libatmcore.so $(API_SYSFUNC)/
	cp -f libatmcore.so $(API_STAT)/

	cp -f libconhal.so $(HAL_CON)/
	cp -f libuarthal.so $(HAL_CON)/
	cp -f libfbdhal.so $(HAL_SCREEN)/
	cp -f libhdmihal.so $(HAL_SCREEN)/
	cp -f libmshal.so $(HAL_MOUSE)/
	cp -f libsndhal.so $(HAL_SOUND)/
	cp -f libethal.so $(HAL_ETHERNET)/
	cp -f libsimhal.so $(HAL_GSM)/
	cp -f libpwrhal.so $(HAL_SYSTEM)/

	cp -f libwhike.so $(AUTUMN_LIB)/

	find $(MODULE) -maxdepth 1 -type f -name '*.ko' -exec cp -f {} $(MODULES)/ \;

	@if [ -f Lat38-VGA32x16.psf ]; then cp -f Lat38-VGA32x16.psf $(FONTS)/; fi

br_build:
	bash -c 'cd $(BR_ROOT)/br-external && source envsetup.sh && set_target mqpro && m all'

clean:
	rm -f session init sysui asurfd modemd AutumnGenericLowLMgr
	rm -f *.so
	rm -f autumn-kexec autumn-atxdb autumn-sudo
	rm -f $(MODULE)/*.ko
	rm -f $(MODULE)/*.o
	rm -f $(MODULE)/*.mod
	rm -f $(MODULE)/*.mod.c
	rm -f $(MODULE)/modules.order
	rm -f $(MODULE)/Module.symvers
	rm -f $(MODULE)/Kbuild
