#!/bin/bash


BR_DIR=$(pwd)
HOST_BIN="$BR_DIR/output/host/bin"
CROSS_CXX="$HOST_BIN/riscv64-linux-g++"
SYSROOT="$BR_DIR/output/host/riscv64-buildroot-linux-gnu/sysroot"

BASE_PATH="/your/path/to/src"
TARGET="$BASE_PATH/AutumnCore"

$CROSS_CXX \
"$BASE_PATH/acore.c" \
"$BASE_PATH/atmhal.c" \
-o "$TARGET" \
-I"$BASE_PATH/" \
--sysroot="$SYSROOT" \
-O3 -lpthread -lrt -lavformat -lavcodec -lswscale -lavutil -lm -lz
