#!/bin/bash

QEMU_BIN="/usr/bin/qemu-system-riscv64"

$QEMU_BIN \
    -M virt \
    -cpu rv64 \
    -m 6G \
    -smp 3 \
    -object memory-backend-ram,id=mem0,size=6G \
    -numa node,nodeid=0,cpus=0-2,memdev=mem0 \
    -kernel output/images/Image \
    -initrd output/images/rootfs.cpio \
    -append "root=/dev/ram0 console=tty1 console=ttyS0 vt.global_cursor_default=0 video=virtio_fb:480x800-32 rdinit=/sbin/init panic=5 loglevel=7 snd_hda_intel.bdl_pos_adj=128" \
    -device virtio-gpu-pci,xres=480,yres=800 \
    -device virtio-tablet-pci \
    -audiodev sdl,id=snd0,timer-period=2000 \
    -device intel-hda -device hda-duplex,audiodev=snd0 \
    -rtc base=localtime,clock=vm \
    -display sdl,gl=off \
    -serial mon:stdio
