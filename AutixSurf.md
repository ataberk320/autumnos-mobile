What is AutixSurf?
AutixSurf is a lightweight, high-performance Surface Manager and Display Server specifically engineered for AutumnOS. Designed as a modern alternative to heavy window managers like SurfaceFlinger or complex Wayland implementations, AutixSurf is optimized for resource-constrained embedded systems.

Key Technical Features
DRM Backend: Operates at the kernel level via the Direct Rendering Manager (DRM) for direct hardware access.

Hardware Acceleration (G2D): Fully utilizes the Allwinner D1s G2D hardware accelerator, offloading graphical tasks from the CPU to boost efficiency.

Memory Efficiency: Architected for devices with 64MB to 128MB of RAM, ensuring a smooth 60 FPS user interface without memory exhaustion.

Cross-Library Support: Seamlessly handles buffers for LVGL, SDL/SDL2, and even raw video streams through FFmpeg.
