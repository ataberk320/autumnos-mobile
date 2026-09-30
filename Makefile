CROSS_COMPILE = /home/ataberk/mangopi-mqpro-br/output/mqpro/host/bin/riscv64-buildroot-linux-gnu-gcc
SYSROOT       = ~/mangopi-mqpro-br/output/mqpro/host/riscv64-buildroot-linux-gnu/sysroot
TINYALSA_INC = ~/mangopi-mqpro-br/mqpro/output/build/tinyalsa-2.0.0/include
MOD_CC = ~/mangopi-mqpro-br/output/mqpro/host/bin/riscv64-buildroot-linux-gnu-

KDIR ?= /home/ataberk/mangopi-mqpro-br/output/mqpro/build/linux-custom

CFLAGS = -I$(SYSROOT)/usr/include/freetype2 -I$(SYSROOT)/usr/include/libpng16 -I$(SYSROOT)/usr/include -I$(TINYALSA_INC) -I$(SYSROOT)/usr/include/libdrm -I$(SYSROOT)/usr/include/SDL2 -I$(SYSROOT)/usr/include/harfbuzz -I$(SYSROOT)/usr/include/cjson -rdynamic -fPIC
LDFLAGS += -L$(SYSROOT)/usr/lib -L. -rdynamic -Wl,-E

obj-m += s800modem.o avinput.o
obj-y += a_panicsod.o tabloader.o coder.o

all: modules hal init session api daemon autumn_lib autumn-tools sysui mkdir mv br_build

hal: libconhal.so libfbdhal.so libmshal.so libsndhal.so libethal.so libpwrhal.so libuarthal.so libsimhal.so libhdmihal.so

api: libfbdev.so libatmio.so libwidget.so libatmsndec.so libatmeth.so libatmdial.so libatmui.so libatmchtn.so libatmjson.so libatmnls.so libpalette.so libatmasurf.so libatmssl.so libatmvidec.so libaction.so libatmtask.so

autumn_lib: libwhike.so

daemon: asurfd modemd AutumnGenericLowLMgr

autumn-tools: autumn-kexec autumn-atxdb autumn-sudo

modules: 
	$(MAKE) -C $(KDIR) ARCH=riscv CROSS_COMPILE=$(MOD_CC) M=$(PWD) modules

asurfd: asurfd.c
	$(CROSS_COMPILE) $(CFLAGS) asurfd.c -o asurfd $(LDFLAGS) -ldrm

AutumnGenericLowLMgr: AutumnGenericLowLMgr.c
	$(CROSS_COMPILE) $(CFLAGS) -rdynamic -Wl,-E -Wl,-undefined,dynamic_lookup AutumnGenericLowLMgr.c -o AutumnGenericLowLMgr $(LDFLAGS) -ldrm -lfreetype

modemd: modemd.c
	$(CROSS_COMPILE) $(CFLAGS) modemd.c -o modemd $(LDFLAGS) -ldrm

libfbdev.so: libfbdev.c
	$(CROSS_COMPILE) $(CFLAGS) -shared -fPIC -Wl,-E -Wl,-undefined,dynamic_lookup libatmnls.c lpngsysc.c libfbdev.c -o libfbdev.so $(LDFLAGS) -lpng -lfreetype -ldl -lgif -ldrm -lharfbuzz

session: session.c
	$(CROSS_COMPILE) $(CFLAGS) -rdynamic loader.c tabldioctl.c sigf.c session.c inputd.c avm.c libatmchtn.c $(LDFLAGS) -o session -lgif  -ldl -lfreetype -lpthread

clean:
	rm -rf session init modemd asurfd autumn-atxdb autumn-sudo autumn-kexec sysui *.so *.atm *.ko *.mod.c *.mod.o *.mod *.o

libwhike.so: libwhike.c
	$(CROSS_COMPILE) -shared -fPIC libwhike.c -o libwhike.so $(LDFLAGS)

autumn-kexec: autumn-kexec.c
	$(CROSS_COMPILE) $(CFLAGS) autumn-kexec.c -o autumn-kexec $(LDFLAGS)

autumn-atxdb: autumn-atxdb.c
	$(CROSS_COMPILE) $(CFLAGS) autumn-atxdb.c -o autumn-atxdb $(LDFLAGS) -lutil

autumn-sudo: autumn-sudo.c
	$(CROSS_COMPILE) $(CFLAGS) autumn-sudo.c -o autumn-sudo $(LDFLAGS)

libatmio.so: libatmio.c
	$(CROSS_COMPILE) -shared -fPIC libatmio.c -o libatmio.so $(LDFLAGS) -lgif -lpng

libatmtask.so: libatmtask.c
	$(CROSS_COMPILE) -shared -fPIC libatmtask.c -o libatmtask.so $(LDFLAGS)

libaction.so: libaction.c
	$(CROSS_COMPILE) $(CFLAGS) -shared -fPIC libaction.c -o libaction.so $(LDFLAGS) -lfreetype

libatmui.so: libatmui.c
	$(CROSS_COMPILE) $(CFLAGS) -shared -fPIC -Wl,-E -Wl,-undefined,dynamic_lookup libatmui.c -o libatmui.so $(LDFLAGS) -lpng -lfreetype -ldl -lgif -ldrm -lm

libconhal.so: console.c
	$(CROSS_COMPILE) -shared -fPIC console.c -o libconhal.so $(LDFLAGS)

libatmvidec.so: libatmvidec.c
	$(CROSS_COMPILE) -shared -fPIC libmp4sysc.c libatmvidec.c -o libatmvidec.so $(LDFLAGS)

libpalette.so: libpalette.c
	$(CROSS_COMPILE) $(CFLAGS) -shared -fPIC libpalette.c -o libpalette.so $(LDFLAGS) -ldrm

libmshal.so: input.c
	$(CROSS_COMPILE) -shared -fPIC input.c -o libmshal.so $(LDFLAGS)

libatmssl.so: libatmtls.c
	$(CROSS_COMPILE) $(CFLAGS) -shared -fPIC libatmtls.c -o libatmssl.so $(LDFLAGS)

libatmchtn.so: libatmchtn.c
	$(CROSS_COMPILE) -shared -fPIC libatmchtn.c -o libatmchtn.so $(LDFLAGS)

libpwrhal.so: power.c
	$(CROSS_COMPILE) -shared -fPIC power.c -Wl,-E -o libpwrhal.so $(LDFLAGS)

libatmeth.so: libatmeth.c
	$(CROSS_COMPILE) $(CFLAGS) -shared -fPIC -Wl,-E libatmeth.c -o libatmeth.so -lfreetype 

libwidget.so: widget.c
	$(CROSS_COMPILE) $(CFLAGS) -shared -fPIC widget.c -o libwidget.so $(LDFLAGS) -lfreetype

libatmjson.so: libatmjson.c
	$(CROSS_COMPILE) $(CFLAGS) -shared -fPIC libatmjson.c -o libatmjson.so $(LDFLAGS) -lcjson

libatmasurf.so: libatmasurf.c
	$(CROSS_COMPILE) $(CFLAGS) -shared -fPIC libatmasurf.c -o libatmasurf.so $(LDFLAGS) -ldrm

libatmnls.so: libatmnls.c
	$(CROSS_COMPILE) $(CFLAGS) -shared -fPIC libatmnls.c -o libatmnls.so $(LDFLAGS) -lharfbuzz

libsndhal.so: sound.c
	$(CROSS_COMPILE) $(CFLAGS) $(LDFLAGS) -shared -fPIC sound.c -o libsndhal.so $(LDFLAGS)

libethal.so: etht.c
	$(CROSS_COMPILE) -shared -fPIC -Wl,-E etht.c -o libethal.so $(LDFLAGS)

libuarthal.so: uart.c
	$(CROSS_COMPILE) $(CFLAGS) -shared -fPIC -Wl,-E uart.c -o libuarthal.so $(LDFLAGS)

libatmdial.so: libatmdial.c
	$(CROSS_COMPILE) $(CFLAGS) -shared -fPIC -Wl,-E libatmdial.c -o libatmdial.so $(LDFLAGS)

libsimhal.so: modem.c
	$(CROSS_COMPILE) $(CFLAGS) -shared -fPIC -Wl,-E modem.c -o libsimhal.so $(LDFLAGS)

libatmsndec.so: libatmsndec.c
	$(CROSS_COMPILE) $(CFLAGS) $(LDFLAGS) -shared -fPIC libatmsndec.c -o libatmsndec.so $(LDFLAGS) -lm -ltinyalsa 

init: init.c
	$(CROSS_COMPILE) -static sigf.c init.c -o init

libfbdhal.so: fb_dev.c
	$(CROSS_COMPILE) $(CFLAGS) -shared -fPIC drm.c fb_dev.c -o libfbdhal.so $(LDFLAGS)

libhdmihal.so: hdmi.c
	$(CROSS_COMPILE) $(CFLAGS) -shared -fPIC hdmi.c -o libhdmihal.so $(LDFLAGS) -lrt -lm -ldrm

sysui: sysui.c
	$(CROSS_COMPILE) $(CFLAGS) -rdynamic -Wl,-E -Wl,-undefined,dynamic_lookup loader.c tabldioctl.c pscrfont.c sysui.c $(LDFLAGS) -o sysui -ldrm

flappy_bird:
	 $(CROSS_COMPILE) $(CFLAGS) TheAutumnsBird/rend.c TheAutumnsBird/logic.c ASurf.c TheAutumnsBird/main.c -o TheAutumnsBird.atm $(LDFLAGS) -latmio -latmchtn -lpthread -latmasurf -ldrm -lpalette
	 cp TheAutumnsBird/assets/ -r ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/share/games/theautumnsbird/assets/



mkdir:
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/ipc
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/autumn
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/ui
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/media
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/io
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/net
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/sysfunc
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/stat
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/mouse
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/sound
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/screen
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/ethernet
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/ethernet/gsm
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/system
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/con
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/share/games
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/bin/
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/bin/
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/sbin/
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/sbin/autumn-tools
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/lib/modules
	mkdir -p ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/share/autumncfonts

mv: 
	cp init ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/sbin/
	cp autumn-kexec ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/sbin/autumn-tools/
	cp autumn-atxdb ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/sbin/autumn-tools/
	cp autumn-sudo ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/sbin/autumn-tools/
	cp session ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/bin/
	cp libfbdev.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/ui/	
	cp libatmnls.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/ui/
	cp libatmasurf.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/ui/
	cp libatmjson.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/net/
	cp libatmssl.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/net/
	cp libatmio.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/io/
	cp libconhal.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/con/
	cp libuarthal.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/con/
	cp libfbdhal.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/screen/
	cp libhdmihal.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/screen/
	cp libmshal.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/mouse/
	cp libwidget.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/ui/
	cp libatmdial.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/ui/
	cp libatmui.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/ui/
	cp libpalette.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/ui/
	cp libsndhal.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/sound/
	cp libethal.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/ethernet/
	cp libsimhal.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/ethernet/gsm
	cp libatmsndec.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/media/
	cp libatmvidec.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/media/
	cp libpwrhal.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/hal/system/
	cp libatmeth.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/net/
	cp libaction.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/sysfunc/
	cp libatmtask.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/stat/
	cp libatmchtn.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/api/ipc/
	cp libwhike.so ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/lib/autumn/
	cp *.ko ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/lib/modules/
	cp asurfd ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/bin/
	cp modemd ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/bin/
	cp AutumnGenericLowLMgr ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/bin/
	cp sysui ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/bin/
	cp Lat38-VGA32x16.psf ~/mangopi-mqpro-br/br-external/board/mqpro/overlay/usr/share/autumncfonts/

br_build:
	@bash -c "cd ~/mangopi-mqpro-br/br-external && source envsetup.sh && set_target mqpro && m all && cd ~/atm_aarch64"
