<img width="148" height="148" alt="1000005253-removebg-preview" src="https://github.com/user-attachments/assets/cfffca9c-ce59-412f-b2d0-dea410e5b420" />

 
# **AutumnOS: RISC-V version**

Originally started as a general OS research on r/osdev, AutumnOS has undergone a significant architectural pivot to meet the demands of modern mobile computing. To achieve maximum hardware compatibility, security, and a rich application ecosystem, the project has been rebuilt as a Linux-based Mobile Operating System.
​By transitioning from a custom microkernel approach to a highly optimized Linux kernel, AutumnOS now leverages advanced process management, robust networking stacks, and extensive driver support. Targeting RISC-V mobile platforms, AutumnOS aims to provide a seamless, open-source mobile experience that bridges the gap between lightweight hardware and handheld efficiency.

Technical information:


- evdev/indev

- RISC-V Allwinner D1s and QEMU(Virtio) support

- DongshanPi/Lichee RV or RV Dock/Mango Pi/Nezha/ClockworkPi/Devterm, multi platform support

- Doom and LTris support

- SDL and SDL2 support

- links support

- ffmpeg and ffplay video player and sound support

- GPIO reader

- LVGL graphics library

- http support

- Virtio support

- Reading battery percent from BAT0

- Reading temperature

- Autumn Core, HAL, API

- sudo (root password)

- RTC support

- dropbear sshd and IP protections

- Camera support(USB/V4L2/UVC etc.)

- g2d 2D/DRM accelerator

- libjpg, libpng support

- fbv, fbtest etc. support

- nano for buildroot terminal

- wget and curl support

- live weather clock widget

- Goldfish support

- Theme manager (Coming soon)

- Watchdog script(Software and hardware watchdog)

- AutixSurf(SurfaceFlinger alternative, DRM/fbdev)

- fbdev/drm/sdl/sdl2

- DNS setup on boot (nameserver 8.8.8.8)

- Kivy and Python support(cython, pip, python3)

- udhcpc

- Timezone data
___________________________

# **Easter eggs**

* Glitch panic (console will open when an a kernel panic occurs and it makes UI glitch!/only watchdog.sh panics!)
* Michael Scott on weather clock widget

  <img width="220" height="225" alt="image-00025-removebg-preview" src="https://github.com/user-attachments/assets/cb29f0fe-2072-4da0-9d36-08c14f316dd6" />

--------------------------
To compile all image: make -j$(nproc)

Carefully read the instructions: 
- /rootfs_overlay/etc/init.d/S99_instruction.txt
- /rootfs_overlay/usr/bin/sysui_instruction.txt
- /buildroot/logos/logo_instruction.txt
- /package_config/config_instruction.txt
- /systemui/ui_instruction.txt
- /boot_logo/boot_anim_instructions.txt

Buildroot version: Buildroot 2026.02-84-g088c3602fd-dirty

To compile lvgl, create a folder name lvgl on buildroot/package and move "Config.in" and lvgl.mk files on /buildroot directory on this repository

LVGL Designer: EEZ Studio 0.26.2

WSL version: must be Ubuntu 22.04 LTS

Run chmod +x for watchdog.sh and S99phone!

LVGL version must be 8.4.0!

---------------------------

# **Interfaces and projects inspired me

* LG Optimus VU - early Optimus UI 3.0 (AutumnUI)

![120222-lg-optimus-Vu-01](https://github.com/user-attachments/assets/ed09edf8-8530-4ec0-a455-d0528a0f8f7e)

* Samsung Galaxy S1 swipe lockscreen

![Samsung_Galaxy_S_(Android_2 2 1)_-_Lock_Screen](https://github.com/user-attachments/assets/c6879258-f497-41da-ac5d-0b52cea6d31a)

* Android 2.x

![froyo11 (1)](https://github.com/user-attachments/assets/16cb6603-21af-469f-bc49-26765f03a671)

* Windows - BSOD (Glitch panic)

![OIP (2)](https://github.com/user-attachments/assets/8efba672-0282-4c0e-bc69-326372699d2e)

* Android Honeycomb (Autumn_HoloUI)

![Android-Honeycomb](https://github.com/user-attachments/assets/f866e342-5ac2-4a80-b982-8e064b0cb06b)


Update notes:
* systemui will be updated soon when it's completed!
* Added API source code
* Icons will be completed!
