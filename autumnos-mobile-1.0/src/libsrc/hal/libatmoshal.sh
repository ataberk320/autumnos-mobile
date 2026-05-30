#!/bin/bash
set -e

CC="/home/ataberk/buildroot/output/host/bin/riscv64-linux-gcc"
SYSROOT="--sysroot=/home/ataberk/buildroot/output/host/riscv64-buildroot-linux-gnu/sysroot"

$CC -c -fPIC ~/buildroot/atmsrc/atmhal.c \
    -o ~/buildroot/atmsrc/atmhal.o \
    -I. -I./cheaders \
    $SYSROOT \
    -DARCH_SUN4I \
    -O2 -Wall

$CC -shared ~/buildroot/atmsrc/atmhal.o \
    -o ~/buildroot/atmsrc/libatmhal.so \
    $SYSROOT
