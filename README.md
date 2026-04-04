Originally started as a general OS research on r/osdev, AutumnOS has undergone a significant architectural pivot to meet the demands of modern mobile computing. To achieve maximum hardware compatibility, security, and a rich application ecosystem, the project has been rebuilt as a Linux-based Mobile Operating System.
​By transitioning from a custom microkernel approach to a highly optimized Linux kernel, AutumnOS now leverages advanced process management, robust networking stacks, and extensive driver support. Targeting powerful ARM and RISC-V mobile platforms, AutumnOS aims to provide a seamless, open-source mobile experience that bridges the gap between desktop-class multitasking and handheld efficiency.

Technical information:
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

g2d 2D or DRM (NEW) accelerator

libjpg, libpng support

fbv, fbtest etc. support

nano for buildroot terminal

wget and curl support (NEW)

live weather clock widget (NEW)

Goldfish support

Theme manager (Coming soon)




To compile all image: make -j3

