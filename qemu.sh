#!/bin/bash

KERNEL="output/images/Image"
INITRD="output/images/rootfs.cpio"

sudo qemu-system-riscv64 -M virt -cpu rv64 -smp 2 -m 2G \
    -kernel $KERNEL \
    -initrd $INITRD \
    -append "root=/dev/ram0 console=tty1 console=ttyS0 fbcon=map:0 video=virtio-gpu:480x800 rdinit=/sbin/init panic=5 loglevel=7" \
    -device virtio-gpu-pci,xres=480,yres=800 \
    -device virtio-keyboard-pci \
    -device qemu-xhci,id=usb_bus \
    -device usb-tablet,bus=usb_bus.0 \
    -device usb-host,vendorid=0x174F,productid=0x118D \
    -display sdl,gl=on \
    -serial stdio \
    -netdev user,id=net0 -device virtio-net-pci,netdev=net0
