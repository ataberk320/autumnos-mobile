#!/bin/bash

set -e
echo "Downloading Buildroot..."
echo "[AutumnOS]: Starting build process..."

echo "Building sysui..."
/home/ataberk/buildroot/output/host/bin/riscv64-linux-gcc -o sysui \
    atmsrc/ui/*.c \
    $(find /home/ataberk/buildroot/output/build/lvgl-8.3.11/src -name "*.c") \
    $(find /home/ataberk/buildroot/output/build/lv_drivers-v8.3.0 -name "*.c" ! -name "lv_drv_conf_template.c") \
    -I. \
    -I./ui \
    -I/home/ataberk/buildroot/output/build/lvgl-8.3.11 \
    -I/home/ataberk/buildroot/output/build \
    -I/home/ataberk/buildroot/output/build/lv_drivers-v8.3.0/display \
    -I/home/ataberk/buildroot/output/build/lv_drivers-v8.3.0/indev \
    -I/home/ataberk/buildroot/output/staging/usr/include/freetype2 \
    -L/home/ataberk/buildroot/output/staging/usr/lib \
    -lfreetype -lpthread -lm \
    -lavcodec -lavformat -lavutil -lswscale \
    -D LV_CONF_INCLUDE_SIMPLE \
    -D LV_LVGL_H_INCLUDE_SIMPLE \
    -D LV_DRV_CONF_INCLUDE_SIMPLE \
    -D USE_FBDEV=1 \
    -D USE_EVDEV=1 \
    -D LV_USE_FREETYPE=1 \
    -D LV_CONF_PATH="ui/lv_conf.h"

echo "Building procterm..."
~/buildroot/output/host/bin/riscv64-linux-g++ -o ~/buildroot/board/autix/generic/rootfs_overlay/usr/bin/procterm \
    ~/buildroot/atmsrc/ProcTerminater.c \
    ~/buildroot/atmsrc/acoreutils.c


echo "Building ASComposer..."
~/buildroot/output/host/bin/riscv64-linux-g++ -o ~/buildroot/board/autix/generic/rootfs_overlay/usr/bin/ASComposer \
    ~/buildroot/atmsrc/ASComposer.cpp \
    ~/buildroot/atmsrc/AutumnAPI.c \
    ~/buildroot/atmsrc/atmhal.c \
    ~/buildroot/atmsrc/AutumnAni.c \
    ~/buildroot/atmsrc/AutumnAppSSTimer.c \
    ~/buildroot/atmsrc/AtmDrv_G2D.c \
    -lpthread -lrt -lavformat -lavcodec -lswscale -lavutil -lm -lz


echo "Building AutixSurfDetector..."
/home/ataberk/buildroot/output/host/bin/riscv64-linux-g++ \
    ~/buildroot/atmsrc/AutixSurfDetect.cpp \
    ~/buildroot/atmsrc/AutixSurfFB0.cpp \
    ~/buildroot/atmsrcAutixSurfDRM.cpp \
    -o ~/buildroot/board/autix/generic/rootfs_overlay/usr/bin/AutixSurfDetector \
    --sysroot=/home/ataberk/buildroot/output/host/riscv64-buildroot-linux-gnu/sysroot \
    -I/home/ataberk/buildroot/output/host/riscv64-buildroot-linux-gnu/sysroot/usr/include/libdrm \
    -L/home/ataberk/buildroot/output/host/riscv64-buildroot-linux-gnu/sysroot/usr/lib \
    -ldrm -lrt


echo "Building AutumnCore..."
/home/ataberk/buildroot/output/host/bin/riscv64-linux-gcc \
    ~/buildroot/atmsrc/acore.c \
    ~/buildroot/atmsrc/acoreutils.c \
    ~/buildroot/atmsrc/AutumnIO.c \
    ~/buildroot/atmsrc/AutumnSyscall.c \
    ~/buildroot/atmsrc/atmhal.c \
    ~/buildroot/atmsrc/AtmDrv_G2D.c \
    ~/buildroot/atmsrc/AutumnAPI.c \
    ~/buildroot/atmsrc/AutumnExternModem.c \
    -o ~/buildroot/board/autix/generic/rootfs_overlay/usr/bin/AutumnCore \
    --sysroot=/home/ataberk/buildroot/output/host/riscv64-buildroot-linux-gnu/sysroot \
    -lpthread -lrt -lavformat -lavcodec -lswscale -lavutil -lm -lz

echo "Building CoreTerminal..."
/home/ataberk/buildroot/output/host/bin/riscv64-linux-gcc \
    ~/buildroot/atmsrc/AutumnCoreUtilsTerminal.c \
    ~/buildroot/atmsrc/acoreutils.c \
    ~/buildroot/atmsrc/AutumnIO.c \
    ~/buildroot/atmsrc/AutumnSyscall.c \
    ~/buildroot/atmsrc/AutumnAPI.c \
    ~/buildroot/atmsrc/atmhal.c \
    -o ~/buildroot/board/autix/generic/rootfs_overlay/usr/bin/CoreTerminal \
    --sysroot=/home/ataberk/buildroot/output/host/riscv64-buildroot-linux-gnu/sysroot \
    -lpthread -lrt -lavformat -lavcodec -lswscale -lavutil -lm -lz

echo "[AutumnOS]: Build completed successfully."
echo "[AutumnOS]: Starting create Buildroot disk image...

make -j3
