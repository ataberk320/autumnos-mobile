#!/bin/bash


KERNEL="output/images/Image"
INITRD="output/images/rootfs.cpio"


sudo qemu-system-riscv64 -M virt -cpu rv64 -smp 4 -m 2G \
    -kernel "$KERNEL" \
    -initrd "$INITRD" \
    -append "root=/dev/ram0 console=tty1 console=ttyS0 fbcon=map:0 video=virtio-gpu:480x800 rdinit=/sbin/init panic=5 loglevel=7 virtio_gpu.modeset=1" \
    -device virtio-gpu-pci,xres=480,yres=800,edid=on \
    -device virtio-keyboard-pci \
    -device virtio-mouse-pci,id=input0 \
    -display sdl,gl=on \
    -serial stdio \
    -serial pty \
    -netdev user,id=net0 -device virtio-net-pci,netdev=net0
