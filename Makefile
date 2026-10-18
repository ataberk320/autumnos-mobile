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

CFLAGS := -O2 -Wall -fPIC --sysroot=$(SYSROOT) \
          -I$(SYSROOT)/usr/include/freetype2 \
          -I$(SYSROOT)/usr/include/libpng16 \
          -I$(SYSROOT)/usr/include \
          -I$(SYSROOT)/usr/include/libdrm \
          -I$(SYSROOT)/usr/include/harfbuzz \
          -I$(SYSROOT)/usr/include/cjson \
          -rdynamic

LDFLAGS := -shared --sysroot=$(SYSROOT) -L$(SYSROOT)/usr/lib -rdynamic -Wl,-E

API_DIR := $(OVERLAY)/usr/lib/api
HAL_DIR := $(OVERLAY)/usr/lib/hal

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
	mkdir -p $(API_DIR)
	mkdir -p $(HAL_DIR)
	mkdir -p $(AUTUMN_LIB) $(BIN) $(SBIN) $(TOOLS) $(USR_BIN) $(MODULES) $(FONTS)

session:
	$(CC) $(CFLAGS) $(SRC)/loader.c $(SRC)/tabldioctl.c $(SRC)/sigf.c $(SRC)/session.c $(SRC)/pthread/inputd.c $(SRC)/pthread/avm.c $(API)/libatmchtn.c -o session $(LDFLAGS) -lpthread -lgif -lfreetype -ldl

init:
	$(CC) $(SRC)/sigf.c $(SRC)/init.c -static -o init

asurfd:
	$(CC) $(CFLAGS) $(SRC)/asurfd.c -o asurfd $(LDFLAGS) -ldrm

modemd:
	$(CC) $(CFLAGS) $(SRC)/modemd.c -o modemd $(LDFLAGS) -ldrm

AutumnGenericLowLMgr:
	$(CC) $(CFLAGS) -Wl,-undefined,dynamic_lookup $(SRC)/AutumnGenericLowLMgr.c -o AutumnGenericLowLMgr $(LDFLAGS) -ldrm -lfreetype

sysui:
	$(CC) $(CFLAGS) -Wl,-undefined,dynamic_lookup $(SRC)/loader.c $(SRC)/tabldioctl.c $(SRC)/pscrfont.c $(SRC)/sysui.c -o sysui $(LDFLAGS) -lpthread -ldrm

libconhal.so:
	$(CC) $(CFLAGS) $(LDFLAGS) $(HAL)/console.c -o libconhal.so

libfbdhal.so:
	$(CC) $(CFLAGS) $(LDFLAGS) $(HAL)/fb_dev.c $(HAL)/drm.c -o libfbdhal.so -ldrm

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
	$(CC) $(CFLAGS) $(LDFLAGS) $(HAL)/hdmi.c -o libhdmihal.so -lrt -lm -ldrm

libatmcore.so:
	$(CC) $(CFLAGS) $(LDFLAGS) -Wl,-undefined,dynamic_lookup \
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
		-o libatmcore.so -lfreetype -lpng -ldl -lgif -ldrm -lharfbuzz -lm -lcjson

libs: libwhike.so

hal: libconhal.so libfbdhal.so libmshal.so libsndhal.so libethal.so libpwrhal.so libuarthal.so libsimhal.so libhdmihal.so

api: libatmcore.so

daemons: asurfd modemd AutumnGenericLowLMgr

tools:
	$(CC) $(CFLAGS) $(SRC)/autumn-kexec.c -o autumn-kexec $(LDFLAGS)
	$(CC) $(CFLAGS) $(SRC)/autumn-atxdb.c -o autumn-atxdb $(LDFLAGS) -lutil
	$(CC) $(CFLAGS) $(SRC)/autumn-sudo.c -o autumn-sudo $(LDFLAGS)

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

	cp -f libatmcore.so $(API_DIR)/

	cp -f libconhal.so $(HAL_DIR)/
	cp -f libuarthal.so $(HAL_DIR)/
	cp -f libfbdhal.so $(HAL_DIR)/
	cp -f libhdmihal.so $(HAL_DIR)/
	cp -f libmshal.so $(HAL_DIR)/
	cp -f libsndhal.so $(HAL_DIR)/
	cp -f libethal.so $(HAL_DIR)/
	cp -f libsimhal.so $(HAL_DIR)/
	cp -f libpwrhal.so $(HAL_DIR)/

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
