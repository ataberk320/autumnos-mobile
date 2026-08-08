<img width="148" height="148" alt="1000005253-removebg-preview" src="https://github.com/user-attachments/assets/cfffca9c-ce59-412f-b2d0-dea410e5b420" />
 
# **The AutumnOS Project**

AutumnOS is an Linux-based operating system project developed for RV64 (RISC-V) architectures.

# Technical Features
* **Dual Audio Subsystem**: An audio subsystem which supports OSS and ALSA
* **ChatTunnel**: A SHM based IPC communication mechanism.
* **AutixSurf**: Layer manager for window manager. (ON WORK)
* **Tabloader**: A kernel driver which loads HAL and API tables globally. (ON WORK)
* **Symbol loader**: An installer for API and HAL libraries before SysUI process.
* **Autumn Virtual Machine**: A memfd based virtual machine for handling heavy processes.
--------------------------------------------------------
# Tested Hardwares 
* Allwinner D1 - MangoPi MQ-Pro (Pink Variation/ v1.4): Stucks on Run /sbin/init as init process log.
* QEMU: Worked perfectly.
--------------------------------------------------------
Working drivers on QEMU VM test:

- DRM
- Dual Audio Subsystem (I tested only OSS)
- Ethernet (No exception but not sure)
---------------------
# Linux Source Code:
This system is based on Linux 6.1 now, To pull original tarball:<br>
<br>
```bash
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.1.tar.xz
tar -xf linux-6.1.tar.xz
cd linux-6.1
```
--------------------
**Licensing:** AutumnOS original components are licensed under GPL-3.0. The Linux kernel (6.1.x) and kernel-derived components retain their respective original licenses, including GPL-2.0-only where applicable.
* **libdrm** — MIT
* **tinyalsa** — BSD-3 Clause
* **U-Boot** — GPL-2.0-or-later
* **libpng** — libpng License
* **libjpeg** — JPEG License
* **giflib** — MIT
* **minimp3** — CC0-1.0
* **minimp4** — CC0-1.0
