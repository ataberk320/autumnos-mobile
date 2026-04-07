BIN_PATH="board/sipeed/lichee_rv_dock/overlay/rootfs_overlay/usr/bin"
COMPILER="output/host/bin/riscv64-linux-g++"
SYSROOT="output/host/riscv64-buildroot-linux-gnu/sysroot"
DRM_INC="$SYSROOT/usr/include/libdrm"

$COMPILER \
    $BIN_PATH/AutixSurfDRM.cpp \
    $BIN_PATH/AutixSurfDetect.cpp \
    -o $BIN_PATH/AutixSurfDetector \
    -I$DRM_INC \
    -ldrm \
    --sysroot=$SYSROOT \
    -lpthread
