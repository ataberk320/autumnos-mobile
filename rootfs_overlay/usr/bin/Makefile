CXX = output/host/bin/riscv64-linux-g++
SYSROOT = output/host/riscv64-buildroot-linux-gnu/sysroot

CXXFLAGS = -I$(SYSROOT)/usr/include/libdrm --sysroot=$(SYSROOT)
LDFLAGS = -ldrm

OVERLAY_DIR = board/sipeed/lichee_rv_dock/overlay/rootfs_overlay/usr/bin

SRCS = $(OVERLAY_DIR)/AutixSurfDRM.cpp \
       $(OVERLAY_DIR)/AutixSurfDetect.cpp \
       $(OVERLAY_DIR)/acore.cpp \
       $(OVERLAY_DIR)/atmhal.c

TARGET = $(OVERLAY_DIR)/AutixSurfDetector

all: $(TARGET)

$(TARGET): $(SRCS)
	@echo "Compiling binaries..."
	$(CXX) $(SRCS) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)
	@echo "Successfully compiled!"

clean:
	@echo "Cleaning..."
	rm -f $(TARGET)
