#!/bin/bash


BR_DIR=$(TOPDIR)/buildroot
HOST_BIN="$BR_DIR/output/host/bin"
CROSS_CXX="$HOST_BIN/riscv64-linux-gcc"
SYSROOT="$BR_DIR/output/host/riscv64-buildroot-linux-gnu/sysroot"

BASE_PATH="$BR_DIR/atm_src"
TARGET="$BASE_PATH/CoreTerminal"

$CROSS_CXX \
"$BASE_PATH/AutumnCoreUtilsTerminal.c" \
"$BASE_PATH/acoreutils.c" \
"$BASE_PATH/AutumnIO.c" \
"$BASE_PATH/AutumnSyscall.c" \
"$BASE_PATH/AutumnAPI.c" \
"$BASE_PATH/atmhal.c" \
-o "$TARGET" \
-I"$BASE_PATH/" \
--sysroot="$SYSROOT" \
-O3 -lpthread -lrt -lavformat -lavcodec -lswscale -lavutil -lm -lz
