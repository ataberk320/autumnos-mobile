Originally started as a general OS research on r/osdev, AutumnOS has undergone a significant architectural pivot to meet the demands of modern mobile computing. To achieve maximum hardware compatibility, security, and a rich application ecosystem, the project has been rebuilt as a Linux-based Mobile Operating System.
​By transitioning from a custom microkernel approach to a highly optimized Linux kernel, AutumnOS now leverages advanced process management, robust networking stacks, and extensive driver support. Targeting RISC-V mobile platforms, AutumnOS aims to provide a seamless, open-source mobile experience that bridges the gap between lightweight hardware and handheld efficiency.

Technical information:


evdev/indev

RISC-V Allwinner D1s and QEMU(Virtio) support

Doom and LTris support

SDL and SDL2 support

links support

ffmpeg and ffplay video player and sound support

GPIO reader

LVGL graphics library

http support

Virtio support

Reading battery percent from BAT0

Reading temperature

Autumn Core, HAL, API(updated)

sudo (root password)

RTC support

dropbear sshd and IP protections

Camera support(USB/V4L2/UVC etc.)

g2d 2D/DRM (NEW) accelerator

libjpg, libpng support

fbv, fbtest etc. support

nano for buildroot terminal

wget and curl support (NEW)

live weather clock widget (NEW)

Goldfish support

Theme manager (Coming soon)

Watchdog script(Software and hardware watchdog)

AutixSurf(SurfaceFlinger alternative, DRM/fbdev)

fbdev/drm/sdl/sdl2

DNS setup on boot (nameserver 8.8.8.8)

Kivy and Python support(cython, pip, python3)

udhcpc

Timezone data
.
___________________________
To compile all image: make -j$(nproc)

Carefully read the instructions in format .txt!: /rootfs_overlay/etc/init.d/S99_instruction.txt, /rootfs_overlay/usr/bin/sysui_instruction.txt, /buildroot/logos/logo_instruction.txt, /package_config/config_instruction.txt, /systemui/ui_instruction.txt

Buildroot version: Buildroot 2026.02-84-g088c3602fd-dirty

To compile lvgl, create a folder name lvgl on buildroot/package and move "Config.in" and lvgl.mk files on /buildroot directory on this repository

LVGL Designer: EEZ Studio 0.26.2

WSL version: must be Ubuntu 22.04 LTS

systemui will be updated soon when it's completed!

LVGL version must be 8.4.*!
