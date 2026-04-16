#!/bin/bash

BR_DIR=$(pwd)
HOST_BIN="$BR_DIR/output/host/bin"
CROSS_CXX="$HOST_BIN/riscv64-linux-g++"
SYSROOT="$BR_DIR/output/host/riscv64-buildroot-linux-gnu/sysroot"

BASE_PATH="board/sipeed/lichee_rv_dock/overlay/rootfs_overlay/usr/bin"
TARGET="$BASE_PATH/AutixSurfDetector"

$CROSS_CXX \
"$BASE_PATH/AutixSurfDetect.cpp" \
"$BASE_PATH/AutixSurfFB0.cpp" \
"$BASE_PATH/AutixSurfDRM.cpp" \
-o "$TARGET" \
--sysroot="$SYSROOT" \
-I"$SYSROOT/usr/include/libdrm" \
-O3 -ldrm -lrt
