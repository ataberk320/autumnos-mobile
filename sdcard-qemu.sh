#!/bin/bash

SDCARD="output/images/sdcard.img"
KERNEL="output/images/Image"

sudo qemu-system-riscv64 -M virt -cpu rv64 -smp 2 -m 2G \
    -kernel $KERNEL \
    -drive file=$SDCARD,format=raw,id=hd0 \
    -device virtio-blk-device,drive=hd0 \
    -append "root=/dev/vda rw console=tty1 console=ttyS0 fbcon=map:0 video=virtio-gpu:480x800 panic=5 loglevel=7" \
    -device virtio-gpu-pci,xres=480,yres=800 \
    -device virtio-keyboard-pci \
    -device qemu-xhci,id=usb_bus \
    -device usb-tablet,bus=usb_bus.0 \
    -display sdl,gl=on \
    -serial stdio \
    -netdev user,id=net0 -device virtio-net-pci,netdev=net0
