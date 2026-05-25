<img width="148" height="148" alt="1000005253-removebg-preview" src="https://github.com/user-attachments/assets/cfffca9c-ce59-412f-b2d0-dea410e5b420" />
Version v1.0
 
# **AutumnOS v1.0**

Originally started as a general OS research on r/osdev, AutumnOS has undergone a significant architectural pivot to meet the demands of modern mobile computing. To achieve maximum hardware compatibility, security, and a rich application ecosystem, the project has been rebuilt as a Linux-based Mobile Operating System.
​By transitioning from a custom microkernel approach to a highly optimized Linux kernel, AutumnOS now leverages advanced process management, robust networking stacks, and extensive driver support. Targeting RISC-V mobile platforms, AutumnOS aims to provide a seamless, open-source mobile experience that bridges the gap between lightweight hardware and handheld efficiency.

# Technical information:
🗂 File system:
   - initramfs
   - ext2/3/4 filesystem

🌐 Connection:
   - FIFO Pipe
   - Unix Domain Socket
   - E1000/VLAN/eth0
   - OpenSSL
   - http support
   - wget and curl
   - dropbear sshd and iproute2 (ip addr, ip link set up, ip route)
   - udhcpc
   - Timezone data
   - libjsip (SIP call)
   - GitBrowser - a GitHub port
   - Pull repo with git
   - ca-certificates
   - netsurf
   - DNS setup (nameserver 8.8.8.8)
   - OTA Software Update

🌄 Graphics:
   - SDL and SDL2
   - LVGL graphics library
   - Allwinner G2D
   - DRM accelerator with KMS
   - libjpg, libpng, freetype
   - AutumnGL (GLES 3.0 based)
   - fbv, fbtest
   - AutixSurf Display Server
   - CedarX
💽 Multimedia:
   - Qt6 multimedia (alsa, ffmpeg, gstreamer)
   - ffmpeg and ffplay video player and sound
   - mpg123
   - gst1/gstreamer
   - Camera (USB/V4L2/UVC etc.)

🔊 Sound driver:
   - Intel HD Audio/AC97
   - alsa/pulseaudio

⚙️ External drivers:
   - evdev/indev
   - USB Gadget (Serial port)
   - USB OHCI/XHCI/HID drivers
   - PCI device
   - HDMI and sun4i Resistive Touchscreen
   - QEMU(Virtio)
   - RTC
   - Watchdog script(Software and hardware watchdog)
   - Goldfish driver support
   - Virtual framebuffer(fb0, fb1)
   - GPIO reader

🗃 Libraries:
   - BusyBox Tools (only boot scripts)
   - AutumnSDK
   - Autumn Core, HAL, API
   - acoreutils (kmsg, kpanic, kassert, kbeat, kmap, ls, rm, cp, mkdir)

🎏 Userspace:
   - AutumnOS CoreUtils Terminal
   - nano for Buildroot terminal
   - sudo wrapper (atmsu_root)
   - Resource managing
   - Multilanguage
   - Multitasking
   - live weather clock widget
   - Theme manager (Coming soon)

📄 Logging/debugging: 
   - LVGL printf log output
   - strace for memory access debug

🧮 Games:<br>
   - chocolate-doom<br>
   - LTris<br>

👾 Coding:<br>
   - Python<br>
   - pip, python3<br>

🗄 Package manager:<br>
   - Autumn Store<br>
   - insmod<br>
   - modprobe<br>
.
___________________________

# **Easter eggs**

* Glitch panic (console will open when an a kernel panic occurs and it makes UI glitch!/only watchdog.sh panics!)
* Michael Scott on weather clock widget
* Gravity 0 (floating icons on home page)
* Kernel hacking (sysrq)
* AutumnOS Bare Idle Mode
--------------------------
# Compiling
To compile all image: make -j$(nproc). For improve compiling performance use ```make -j$(nproc)/2```

**Important! Carefully read the instructions:** 
[**All Instructions**](https://www.github.com/ataberk320/autumnos-mobile/tree/main/autumnos-mobile-1.0/instructions/instructions.txt)

Buildroot version: Buildroot 2026.02-84-g088c3602fd-dirty <br>
To pull this version: <br>
```git clone https://github.com/buildroot/buildroot.git -> cd buildroot -> git checkout 088c3602fd```

LVGL Designer: EEZ Studio 0.26.2 <br>
EEZ Studio setup link: https://drive.google.com/file/d/1Nfeg9FtOeK48H4LRhqOBRV9haR9v_n8W/view?usp=sharing

To get Qt Creator: [**QT6 link**](https://drive.google.com/file/d/1P_o6vA0y19AfJXvIR_mIG1IByxjvckpz/view?usp=sharing) <br>
I bypassed -j9 for Qt6 (by BR2_JLEVEL) <br>


WSL version: must be Ubuntu 22.04 LTS

LVGL version must be 8.4.0!

Target architecture: RV64 (RISC-V 64bit)

CPU requirement: 256MB+

Supported SoC's: StarFive JH7110, Allwinner D1 family

---------------------------

# **Interfaces and projects inspired me**

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

* HTC Lock Screen Incoming Call

<img width="1280" height="720" alt="maxresdefault (1)" src="https://github.com/user-attachments/assets/8ee2f6ed-91bf-496a-8bf8-a8c54ce37579" />


------------------------------------------------------------------------------

System screenshots:

[**Screenshots Directory**](https://github.com/ataberk320/autumnos-mobile/tree/main/autumnos-mobile-1.0/screenshots)


------------------------------------------------------------------------------
For Non SoM(System On Module) chips:

* *For Allwinner D1 SoC's:*
[**Customized generic Device Tree Blob for Allwinner D1 by ataberk320**](https://github.com/ataberk320/autumnos-mobile/raw/main/autumnos-mobile-1.0/kernel/DeviceTreeBlob/sun20i-d1-autumn-default.dtb) <br>
* *For Allwinner D1s SoC's:*
[**Allwinner D1s is out of support, for this SoC, use AutumnOS "Through" Edition**](https://github.com/ataberk320/autumnos-rt)

-----------------
# Packages and userspace:
- **AutumnSDK:** App development and SDK environment for AutumnOS<br>
- **acoreutils:** File operations and kmsg like functions. (BusyTools is removed from userspace)<br>
- **ASComposer:** Layer composer for AutixSurf Display Server.<br>
- **AutixSurf:** Our display server which supports DRM/fb0 [**Documentation**](https://www.github.com/ataberk320/autumnos-mobile/tree/main/autumnos-mobile-1.0/AutixSurf.md)<br>
- **AutumnGL**: GLES 3.0 based graphics engine.

---------------------
# **Freedom for developers**
- Endless Life Policity: Developers can use source code (According to GPL-3.0) and write their own Custom ROM and port new features.
- Protected files: If you want to downgrade your AutumnOS version, your data will not be lost thanks to ping pong slot and User data partition on SD card.
