<img width="148" height="148" alt="1000005253-removebg-preview" src="https://github.com/user-attachments/assets/cfffca9c-ce59-412f-b2d0-dea410e5b420" />
 
# **The AutumnOS Project**

AutumnOS is an Linux-based operating system project developed for RV64 (RISC-V) architectures.

# Technical Specifications
----------------------------------------------------
* **Dual Audio Subsystem**: An audio subsystem which supports OSS and ALSA
* **ChatTunnel**: A SHM based IPC communication mechanism.
* **AutixSurf**: Layer manager for window manager. (ON WORK)
* **Tabloader**: A kernel driver which loads HAL and API tables globally. (ON WORK)
* **Symbol loader**: An installer for API and HAL libraries before SysUI process.
* **Autumn Virtual Machine**: A memfd based virtual machine for handling heavy processes.
--------------------------------------------------------
# Tested Hardwares
--------------------------------------------------------
* Allwinner D1 - MangoPi MQ-Pro (Pink Variation/ v1.4): Stucks on Run /sbin/init as init process log.
--------------------------------------------------------
Working drivers on QEMU VM test:
---------------------------------------------------------
- DRM
- Dual Audio Subsystem
- ......
# Linux Source Code:
----------------------------------------------------------
This system is based on Linux 6.1 now, To pull original tarball:
**```bash
wget [https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.1.tar.xz](https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.1.tar.xz)
tar -xf linux-6.1.tar.xz
cd linux-6.1```
