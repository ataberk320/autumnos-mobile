```make
BR_ROOT := /home/ataberk/mangopi-mqpro-br
OUTPUT  := $(BR_ROOT)/output/mqpro

CROSS_COMPILE := $(OUTPUT)/host/bin/riscv64-buildroot-linux-gnu-
CC := $(CROSS_COMPILE)gcc
SYSROOT := $(OUTPUT)/host/riscv64-buildroot-linux-gnu/sysroot
MOD_CC := $(CROSS_COMPILE)

KDIR ?= $(OUTPUT)/build/linux-custom

PROJECT := $(CURDIR)

SRC       := $(PROJECT)/src
API       := $(PROJECT)/API
HAL       := $(SRC)/HAL
PTHREAD   := $(HAL)/pthread
ALLWINNER := $(HAL)/allwinner
MODULE    := $(PROJECT)/kernel/module

OVERLAY := $(BR_ROOT)/br-external/board/mqpro/overlay

OVERLAY_API     := $(OVERLAY)/usr/lib/api
OVERLAY_HAL     := $(OVERLAY)/usr/lib/hal
OVERLAY_AUTUMN  := $(OVERLAY)/usr/lib/autumn
OVERLAY_BIN     := $(OVERLAY)/bin
OVERLAY_SBIN    := $(OVERLAY)/sbin
OVERLAY_USRBIN  := $(OVERLAY)/usr/bin
OVERLAY_MODULES := $(OVERLAY)/lib/modules
OVERLAY_FONTS   := $(OVERLAY)/usr/share/autumncfonts
OVERLAY_GAMES   := $(OVERLAY)/usr/share/games

TINYALSA_INC := $(BR_ROOT)/output/build/tinyalsa-2.0.0/include

CFLAGS := \
	-I$(SYSROOT)/usr/include \
	-I$(SYSROOT)/usr/include/freetype2 \
	-I$(SYSROOT)/usr/include/libpng16 \
	-I$(SYSROOT)/usr/include/libdrm \
	-I$(SYSROOT)/usr/include/SDL2 \
	-I$(SYSROOT)/usr/include/harfbuzz \
	-I$(SYSROOT)/usr/include/cjson \
	-I$(TINYALSA_INC) \
	-I$(SRC) \
	-I$(API) \
	-I$(HAL) \
	-I$(PTHREAD) \
	-I$(ALLWINNER) \
	-I$(MODULE) \
	-rdynamic \
	-fPIC

LDFLAGS := \
	-L$(SYSROOT)/usr/lib \
	-L. \
	-rdynamic \
	-Wl,-E

obj-m += s800modem.o
obj-m += avinput.o

obj-y += a_panicsod.o
obj-y += tabloader.o
obj-y += coder.o

all: modules hal init session api daemon autumn_lib autumn-tools sysui mkdir install br_build

modules:
	$(MAKE) -C $(KDIR) \
		ARCH=riscv \
		CROSS_COMPILE=$(MOD_CC) \
		M=$(MODULE) \
		modules

hal: \
	libconhal.so \
	libfbdhal.so \
	libmshal.so \
	libsndhal.so \
	libethal.so \
	libpwrhal.so \
	libuarthal.so \
	libsimhal.so \
	libhdmihal.so

libconhal.so: $(HAL)/console.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(HAL)/console.c \
		-o $@ \
		$(LDFLAGS)

libfbdhal.so: $(HAL)/fb_dev.c $(HAL)/drm.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(HAL)/drm.c \
		$(HAL)/fb_dev.c \
		-o $@ \
		$(LDFLAGS)

libmshal.so: $(HAL)/input.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(HAL)/input.c \
		-o $@ \
		$(LDFLAGS)

libsndhal.so: $(HAL)/sound.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(HAL)/sound.c \
		-o $@ \
		$(LDFLAGS)

libethal.so: $(HAL)/etht.c
	$(CC) $(CFLAGS) -shared -fPIC -Wl,-E \
		$(HAL)/etht.c \
		-o $@ \
		$(LDFLAGS)

libpwrhal.so: $(HAL)/power.c
	$(CC) $(CFLAGS) -shared -fPIC -Wl,-E \
		$(HAL)/power.c \
		-o $@ \
		$(LDFLAGS)

libuarthal.so: $(HAL)/uart.c
	$(CC) $(CFLAGS) -shared -fPIC -Wl,-E \
		$(HAL)/uart.c \
		-o $@ \
		$(LDFLAGS)

libsimhal.so: $(HAL)/modem.c
	$(CC) $(CFLAGS) -shared -fPIC -Wl,-E \
		$(HAL)/modem.c \
		-o $@ \
		$(LDFLAGS)

libhdmihal.so: $(HAL)/hdmi.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(HAL)/hdmi.c \
		-o $@ \
		$(LDFLAGS) \
		-lrt -lm -ldrm

api: \
	libfbdev.so \
	libatmio.so \
	libwidget.so \
	libatmsndec.so \
	libatmeth.so \
	libatmdial.so \
	libatmui.so \
	libatmchtn.so \
	libatmjson.so \
	libatmnls.so \
	libpalette.so \
	libatmasurf.so \
	libatmssl.so \
	libatmvidec.so \
	libaction.so \
	libatmtask.so

libfbdev.so: $(API)/libfbdev.c $(API)/lpngsysc.c $(API)/libatmnls.c
	$(CC) $(CFLAGS) -shared -fPIC \
		-Wl,-E \
		-Wl,-undefined,dynamic_lookup \
		$(API)/libatmnls.c \
		$(API)/lpngsysc.c \
		$(API)/libfbdev.c \
		-o $@ \
		$(LDFLAGS) \
		-lpng -lfreetype -ldl -lgif -ldrm -lharfbuzz

libatmio.so: $(API)/libatmio.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(API)/libatmio.c \
		-o $@ \
		$(LDFLAGS) \
		-lgif -lpng

libwidget.so: $(API)/widget.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(API)/widget.c \
		-o $@ \
		$(LDFLAGS) \
		-lfreetype

libatmui.so: $(API)/libatmui.c
	$(CC) $(CFLAGS) -shared -fPIC \
		-Wl,-E \
		-Wl,-undefined,dynamic_lookup \
		$(API)/libatmui.c \
		-o $@ \
		$(LDFLAGS) \
		-lpng -lfreetype -ldl -lgif -ldrm -lm

libatmsndec.so: $(API)/libatmsndec.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(API)/libatmsndec.c \
		-o $@ \
		$(LDFLAGS) \
		-lm -ltinyalsa

libatmeth.so: $(API)/libatmeth.c
	$(CC) $(CFLAGS) -shared -fPIC -Wl,-E \
		$(API)/libatmeth.c \
		-o $@ \
		-lfreetype

libatmdial.so: $(API)/libatmdial.c
	$(CC) $(CFLAGS) -shared -fPIC -Wl,-E \
		$(API)/libatmdial.c \
		-o $@ \
		$(LDFLAGS)

libatmchtn.so: $(API)/libatmchtn.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(API)/libatmchtn.c \
		-o $@ \
		$(LDFLAGS)

libatmjson.so: $(API)/libatmjson.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(API)/libatmjson.c \
		-o $@ \
		$(LDFLAGS) \
		-lcjson

libatmnls.so: $(API)/libatmnls.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(API)/libatmnls.c \
		-o $@ \
		$(LDFLAGS) \
		-lharfbuzz

libpalette.so: $(API)/libpalette.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(API)/libpalette.c \
		-o $@ \
		$(LDFLAGS) \
		-ldrm

libatmasurf.so: $(API)/libatmasurf.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(API)/libatmasurf.c \
		-o $@ \
		$(LDFLAGS) \
		-ldrm

libatmssl.so: $(API)/libatmtls.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(API)/libatmtls.c \
		-o $@ \
		$(LDFLAGS)

libatmvidec.so: $(API)/libatmvidec.c $(API)/libmp4sysc.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(API)/libmp4sysc.c \
		$(API)/libatmvidec.c \
		-o $@ \
		$(LDFLAGS)

libaction.so: $(API)/libaction.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(API)/libaction.c \
		-o $@ \
		$(LDFLAGS) \
		-lfreetype

libatmtask.so: $(API)/libatmtask.c
	$(CC) $(CFLAGS) -shared -fPIC \
		$(API)/libatmtask.c \
		-o $@ \
		$(LDFLAGS)

daemon: asurfd modemd AutumnGenericLowLMgr

asurfd: $(SRC)/asurfd.c
	$(CC) $(CFLAGS) \
		$(SRC)/asurfd.c \
		-o $@ \
		$(LDFLAGS) \
		-ldrm

modemd: $(SRC)/modemd.c
	$(CC) $(CFLAGS) \
		$(SRC)/modemd.c \
		-o $@ \
		$(LDFLAGS) \
		-ldrm

AutumnGenericLowLMgr: $(SRC)/AutumnGenericLowLMgr.c
	$(CC) $(CFLAGS) \
		-rdynamic \
		-Wl,-E \
		-Wl,-undefined,dynamic_lookup \
		$(SRC)/AutumnGenericLowLMgr.c \
		-o $@ \
		$(LDFLAGS) \
		-ldrm -lfreetype

session: \
	$(SRC)/session.c \
	$(SRC)/loader.c \
	$(SRC)/tabldioctl.c \
	$(SRC)/sigf.c \
	$(SRC)/inputd.c \
	$(SRC)/avm.c \
	$(API)/libatmchtn.c

	$(CC) $(CFLAGS) \
		-rdynamic \
		$(SRC)/loader.c \
		$(SRC)/tabldioctl.c \
		$(SRC)/sigf.c \
		$(SRC)/inputd.c \
		$(SRC)/avm.c \
		$(API)/libatmchtn.c \
		$(SRC)/session.c \
		-o $@ \
		$(LDFLAGS) \
		-lgif -ldl -lfreetype -lpthread

init: $(SRC)/init.c $(SRC)/sigf.c
	$(CC) -static \
		$(SRC)/sigf.c \
		$(SRC)/init.c \
		-o $@

autumn_lib: libwhike.so

libwhike.so: $(SRC)/libwhike.c
	$(CC) -shared -fPIC \
		$(SRC)/libwhike.c \
		-o $@ \
		$(LDFLAGS)

autumn-tools: autumn-kexec autumn-atxdb autumn-sudo

autumn-kexec: $(SRC)/autumn-kexec.c
	$(CC) $(CFLAGS) \
		$(SRC)/autumn-kexec.c \
		-o $@ \
		$(LDFLAGS)

autumn-atxdb: $(SRC)/autumn-atxdb.c
	$(CC) $(CFLAGS) \
		$(SRC)/autumn-atxdb.c \
		-o $@ \
		$(LDFLAGS) \
		-lutil

autumn-sudo: $(SRC)/autumn-sudo.c
	$(CC) $(CFLAGS) \
		$(SRC)/autumn-sudo.c \
		-o $@ \
		$(LDFLAGS)

sysui: \
	$(SRC)/sysui.c \
	$(SRC)/loader.c \
	$(SRC)/tabldioctl.c \
	$(SRC)/pscrfont.c

	$(CC) $(CFLAGS) \
		-rdynamic \
		-Wl,-E \
		-Wl,-undefined,dynamic_lookup \
		$(SRC)/loader.c \
		$(SRC)/tabldioctl.c \
		$(SRC)/pscrfont.c \
		$(SRC)/sysui.c \
		-o $@ \
		$(LDFLAGS) \
		-ldrm

mkdir:
	mkdir -p $(OVERLAY_API)
	mkdir -p $(OVERLAY_HAL)
	mkdir -p $(OVERLAY_API)/ipc
	mkdir -p $(OVERLAY_AUTUMN)
	mkdir -p $(OVERLAY_API)/ui
	mkdir -p $(OVERLAY_API)/media
	mkdir -p $(OVERLAY_API)/io
	mkdir -p $(OVERLAY_API)/net
	mkdir -p $(OVERLAY_API)/sysfunc
	mkdir -p $(OVERLAY_API)/stat
	mkdir -p $(OVERLAY_HAL)/mouse
	mkdir -p $(OVERLAY_HAL)/sound
	mkdir -p $(OVERLAY_HAL)/screen
	mkdir -p $(OVERLAY_HAL)/ethernet
	mkdir -p $(OVERLAY_HAL)/ethernet/gsm
	mkdir -p $(OVERLAY_HAL)/system
	mkdir -p $(OVERLAY_HAL)/con
	mkdir -p $(OVERLAY_GAMES)
	mkdir -p $(OVERLAY)/usr/bin
	mkdir -p $(OVERLAY)/bin
	mkdir -p $(OVERLAY)/sbin
	mkdir -p $(OVERLAY)/sbin/autumn-tools
	mkdir -p $(OVERLAY_MODULES)
	mkdir -p $(OVERLAY_FONTS)

install: mkdir
	cp init $(OVERLAY_SBIN)/
	cp session $(OVERLAY_BIN)/
	cp autumn-kexec $(OVERLAY_SBIN)/autumn-tools/
	cp autumn-atxdb $(OVERLAY_SBIN)/autumn-tools/
	cp autumn-sudo $(OVERLAY_SBIN)/autumn-tools/
	cp libfbdev.so $(OVERLAY_API)/ui/
	cp libatmnls.so $(OVERLAY_API)/ui/
	cp libatmasurf.so $(OVERLAY_API)/ui/
	cp libwidget.so $(OVERLAY_API)/ui/
	cp libatmdial.so $(OVERLAY_API)/ui/
	cp libatmui.so $(OVERLAY_API)/ui/
	cp libpalette.so $(OVERLAY_API)/ui/
	cp libatmio.so $(OVERLAY_API)/io/
	cp libatmjson.so $(OVERLAY_API)/net/
	cp libatmssl.so $(OVERLAY_API)/net/
	cp libatmeth.so $(OVERLAY_API)/net/
	cp libatmchtn.so $(OVERLAY_API)/ipc/
	cp libatmsndec.so $(OVERLAY_API)/media/
	cp libatmvidec.so $(OVERLAY_API)/media/
	cp libaction.so $(OVERLAY_API)/sysfunc/
	cp libatmtask.so $(OVERLAY_API)/stat/
	cp libconhal.so $(OVERLAY_HAL)/con/
	cp libuarthal.so $(OVERLAY_HAL)/con/
	cp libfbdhal.so $(OVERLAY_HAL)/screen/
	cp libhdmihal.so $(OVERLAY_HAL)/screen/
	cp libmshal.so $(OVERLAY_HAL)/mouse/
	cp libsndhal.so $(OVERLAY_HAL)/sound/
	cp libethal.so $(OVERLAY_HAL)/ethernet/
	cp libsimhal.so $(OVERLAY_HAL)/ethernet/gsm/
	cp libpwrhal.so $(OVERLAY_HAL)/system/
	cp libwhike.so $(OVERLAY_AUTUMN)/
	cp asurfd $(OVERLAY_BIN)/
	cp modemd $(OVERLAY_BIN)/
	cp AutumnGenericLowLMgr $(OVERLAY_BIN)/
	cp sysui $(OVERLAY_USRBIN)/
	cp $(MODULE)/*.ko $(OVERLAY_MODULES)/
	cp $(PROJECT)/Lat38-VGA32x16.psf $(OVERLAY_FONTS)/

br_build:
	@bash -c 'cd $(BR_ROOT)/br-external && source envsetup.sh && set_target mqpro && m all'

clean:
	rm -rf \
		session \
		init \
		modemd \
		asurfd \
		AutumnGenericLowLMgr \
		autumn-atxdb \
		autumn-sudo \
		autumn-kexec \
		sysui \
		*.so \
		*.atm \
		*.ko \
		*.mod.c \
		*.mod.o \
		*.mod \
		*.o
```
