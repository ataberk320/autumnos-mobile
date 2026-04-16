#!/bin/bash


BR_DIR=$(pwd)
HOST_BIN="$BR_DIR/output/host/bin"
CROSS_CC="$HOST_BIN/riscv64-linux-gcc" # .c olduğu için gcc kullanıyoruz
SYSROOT="$BR_DIR/output/host/riscv64-buildroot-linux-gnu/sysroot"

SOURCE_FILE="board/sipeed/lichee_rv_dock/overlay/rootfs_overlay/usr/bin/AutixG2DService.c"
TARGET_BINARY="board/sipeed/lichee_rv_dock/overlay/rootfs_overlay/usr/bin/AutixG2DService"



$CROSS_CC -O3 "$SOURCE_FILE" -o "$TARGET_BINARY" \
--sysroot="$SYSROOT" \
-I"$SYSROOT/usr/include" \
-L"$SYSROOT/usr/lib" \
-lpthread -ldl -lm
