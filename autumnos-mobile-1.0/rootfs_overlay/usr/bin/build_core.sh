#!/bin/bash

BIN_PATH="board/sipeed/lichee_rv_dock/overlay/rootfs_overlay/usr/bin"
COMPILER="output/host/bin/riscv64-linux-g++"
SYSROOT="output/host/riscv64-buildroot-linux-gnu/sysroot"

$COMPILER \
    $BIN_PATH/acore.c \
    $BIN_PATH/atmhal.c \
    -o $BIN_PATH/AutumnCore \
    -I$BIN_PATH/ \
    --sysroot=$SYSROOT \
    -lpthread -lrt -lavformat -lavcodec -lswscale -lavutil -lm -lz
