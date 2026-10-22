SHELL := /bin/bash
.ONESHELL:
.SHELLFLAGS := -eu -o pipefail -c
.DEFAULT_GOAL := all
.DELETE_ON_ERROR:

ifeq ($(IN_GENTOO),1)

PROJECT_NAME := AutumnOS
PROJECT_VERSION := v0.1-alpha
TARGET_PROFILE := casper-nirvana-sb-s100
TARGET_ARCH := x86_64

ROOT_DIR := $(abspath $(CURDIR))
PROJECT := $(ROOT_DIR)/v0.1-alpha
SRC := $(PROJECT)/src
API_SRC := $(PROJECT)/API
HAL_SRC := $(SRC)/HAL
SERVICE_SRC := $(HAL_SRC)/Service
SYSFS_SRC := $(HAL_SRC)/SysFS_Node
MODULE_SRC := $(PROJECT)/kernel/module

BUILD_DIR := $(ROOT_DIR)/build
SOURCES_DIR := $(BUILD_DIR)/sources
OBJ_DIR := $(BUILD_DIR)/obj
BIN_BUILD := $(BUILD_DIR)/bin
LIB_BUILD := $(BUILD_DIR)/lib
OUTPUT_DIR := $(BUILD_DIR)/output
PROFILE_OUTPUT := $(OUTPUT_DIR)/$(TARGET_PROFILE)
STAGING_ROOT := $(PROFILE_OUTPUT)/rootfs

ROOTFS_DIR ?= /
JOBS ?= 4
CC := gcc
MAKE := make
CURL ?= curl
TAR ?= tar
INSTALL := install

CPPFLAGS += -I$(PROJECT)/include -I$(PROJECT)/include/HAL -I$(API_SRC) -I$(HAL_SRC) -I$(SYSFS_SRC)
CFLAGS += -O2 -Wall -Wextra -fPIC
LDFLAGS +=
LDLIBS += -ldl -lpthread -lm

GRAPHICS_LIBS := -ldrm -lfreetype -lpng -lharfbuzz -lgif
JSON_LIBS := -lcjson
AUDIO_LIBS := -lasound

API_NAMES := libatmasurf libatmchtn libatmdial libatmeth \
             libatmio libatmjson libatmnls libatmsndec \
             libatmtask libatmtls libatmui libatmvidec \
             libfbdev libmp4sysc libpalette widget

API_LIBS := $(addprefix $(LIB_BUILD)/api/,$(addsuffix .so,$(API_NAMES)))
WHIKE_LIB := $(LIB_BUILD)/libwhike.so

HAL_NAMES := console drm etht fb_dev hdmi input modem \
             sound uart usb v4l2

HAL_LIBS := $(addprefix $(LIB_BUILD)/hal/lib,$(addsuffix .so,$(HAL_NAMES)))

PROGRAMS := init session asurfd powerd tempsecd
DAEMON_BINS := $(addprefix $(BIN_BUILD)/,$(PROGRAMS))

TOOLS := autumn-atxdb autumn-kexec autumn-sudo
TOOL_BINS := $(addprefix $(BIN_BUILD)/,$(TOOLS))

KERNEL_VERSION := 6.1.176
KERNEL_NAME := linux-$(KERNEL_VERSION)
KERNEL_ARCHIVE := $(SOURCES_DIR)/$(KERNEL_NAME).tar.xz
KERNEL_SRC := $(SOURCES_DIR)/$(KERNEL_NAME)
KERNEL_URL := https://cdn.kernel.org/pub/linux/kernel/v6.x/$(KERNEL_NAME).tar.xz

KERNEL_OUT := $(BUILD_DIR)/kernel-build/$(TARGET_PROFILE)
KERNEL_IMAGE := $(KERNEL_OUT)/arch/x86/boot/bzImage
KERNEL_ARCH := x86
KERNEL_DEFCONFIG := x86_64_defconfig

.PHONY: all help check-tools check-gentoo prepare \
        userspace apis hal daemons tools \
        download-kernel extract-kernel configure-kernel \
        build-kernel modules kernel install install-deps \
        sync-portage info clean distclean

all: check-tools prepare install-deps kernel userspace install info

help:
	@echo "AutumnOS $(PROJECT_VERSION)"
	@echo "Targets: all userspace apis hal daemons tools kernel install install-deps"
	@echo "Maintenance: check-tools check-gentoo info clean distclean"

check-tools:
	@for tool in make gcc curl tar emerge; do \
		command -v "$$tool" >/dev/null 2>&1 || { echo "Missing tool: $$tool"; exit 1; }; \
	done
	@test -x /bin/bash
	@echo "[OK] Gentoo build environment is available."

check-gentoo:
	@test -x /bin/bash
	@command -v emerge >/dev/null 2>&1
	@echo "[OK] Running inside Gentoo."

prepare:
	@mkdir -p "$(SOURCES_DIR)" "$(OBJ_DIR)" "$(BIN_BUILD)" \
		"$(LIB_BUILD)/api" "$(LIB_BUILD)/hal" \
		"$(KERNEL_OUT)" "$(PROFILE_OUTPUT)" "$(STAGING_ROOT)"

install-deps: check-gentoo
	emerge --noreplace --ask=n \
		dev-vcs/git \
		dev-libs/cJSON \
		dev-libs/libxml2 \
		dev-libs/mxml \
		dev-libs/openssl \
		media-libs/freetype \
		media-libs/harfbuzz \
		media-libs/fontconfig \
		media-libs/giflib \
		x11-libs/libdrm \
		media-libs/libpng \
		media-libs/alsa-lib \
		sys-devel/bc \
		sys-devel/bison \
		sys-devel/flex

sync-portage: check-gentoo
	emerge --sync

userspace: kernel apis hal daemons tools

apis hal daemons tools: kernel install-deps

apis: $(API_LIBS) $(WHIKE_LIB)

$(LIB_BUILD)/api/%.so: $(API_SRC)/%.c | prepare
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared "$<" -o "$@" \
		$(LDFLAGS) $(GRAPHICS_LIBS) $(JSON_LIBS) $(LDLIBS)

$(WHIKE_LIB): $(API_SRC)/libwhike.c | prepare
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared "$<" -o "$@" \
		$(LDFLAGS) $(LDLIBS)

hal: $(HAL_LIBS)

$(LIB_BUILD)/hal/lib%.so: $(HAL_SRC)/%.c | prepare
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared "$<" -o "$@" \
		$(LDFLAGS) $(GRAPHICS_LIBS) $(AUDIO_LIBS) $(LDLIBS)

daemons: $(DAEMON_BINS)

$(BIN_BUILD)/init: $(SRC)/init.c $(SRC)/sigf.c | prepare
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o "$@" $(LDFLAGS) $(LDLIBS)

$(BIN_BUILD)/session: $(SRC)/loader.c $(SRC)/tabldioctl.c \
	$(SRC)/sigf.c $(SRC)/session.c $(SRC)/pthread/inputd.c \
	$(SRC)/pthread/avm.c $(API_SRC)/libatmchtn.c | prepare
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o "$@" $(LDFLAGS) $(LDLIBS)

$(BIN_BUILD)/asurfd: $(SRC)/asurfd.c | prepare
	$(CC) $(CPPFLAGS) $(CFLAGS) "$<" -o "$@" $(LDFLAGS) $(LDLIBS)

$(BIN_BUILD)/powerd: $(SERVICE_SRC)/power.c $(SYSFS_SRC)/bat.c | prepare
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o "$@" $(LDFLAGS) $(LDLIBS)

$(BIN_BUILD)/tempsecd: $(SRC)/tempsecd.c $(SRC)/sysrqcmd.c \
	$(SYSFS_SRC)/bat.c | prepare
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o "$@" $(LDFLAGS) $(LDLIBS)

tools: $(TOOL_BINS)

$(BIN_BUILD)/autumn-atxdb: $(SRC)/autumn-atxdb.c | prepare
	$(CC) $(CPPFLAGS) $(CFLAGS) "$<" -o "$@" \
		$(LDFLAGS) -lutil $(LDLIBS)

$(BIN_BUILD)/autumn-kexec: $(SRC)/autumn-kexec.c | prepare
	$(CC) $(CPPFLAGS) $(CFLAGS) "$<" -o "$@" $(LDFLAGS) $(LDLIBS)

$(BIN_BUILD)/autumn-sudo: $(SRC)/autumn-sudo.c | prepare
	$(CC) $(CPPFLAGS) $(CFLAGS) "$<" -o "$@" $(LDFLAGS) $(LDLIBS)

download-kernel: prepare
	@if [ ! -s "$(KERNEL_ARCHIVE)" ]; then
		$(CURL) -fL --retry 3 "$(KERNEL_URL)" -o "$(KERNEL_ARCHIVE)"
	else
		echo "[SKIP] Kernel archive already exists."
	fi

extract-kernel: download-kernel
	@if [ ! -f "$(KERNEL_SRC)/.autumnos-extracted" ]; then
		mkdir -p "$(KERNEL_SRC)"
		$(TAR) -xf "$(KERNEL_ARCHIVE)" \
			--strip-components=1 -C "$(KERNEL_SRC)"
		touch "$(KERNEL_SRC)/.autumnos-extracted"
	else
		echo "[SKIP] Kernel sources already extracted."
	fi

configure-kernel: install-deps extract-kernel
	@mkdir -p "$(KERNEL_OUT)"
	@if [ ! -s "$(KERNEL_OUT)/.config" ]; then
		$(MAKE) -C "$(KERNEL_SRC)" O="$(KERNEL_OUT)" \
			ARCH="$(KERNEL_ARCH)" "$(KERNEL_DEFCONFIG)"
	fi
	@"$(KERNEL_SRC)/scripts/config" --file "$(KERNEL_OUT)/.config" \
		-e MODULES \
		-e GPIOLIB \
		-e MAGIC_SYSRQ \
		--set-val MAGIC_SYSRQ_DEFAULT_ENABLE 0x1f6 \
		-e SERIAL_CORE \
		-e SERIAL_DEV_BUS
	$(MAKE) -C "$(KERNEL_SRC)" O="$(KERNEL_OUT)" \
		ARCH="$(KERNEL_ARCH)" olddefconfig
	@grep -E '^(CONFIG_MODULES|CONFIG_GPIOLIB|CONFIG_MAGIC_SYSRQ|CONFIG_MAGIC_SYSRQ_DEFAULT_ENABLE|CONFIG_SERIAL_CORE|CONFIG_SERIAL_DEV_BUS)=' \
		"$(KERNEL_OUT)/.config" || true

build-kernel: configure-kernel
	$(MAKE) -C "$(KERNEL_SRC)" O="$(KERNEL_OUT)" \
		ARCH="$(KERNEL_ARCH)" -j"$(JOBS)" bzImage modules
	@test -f "$(MODULE_SRC)/Kbuild" -o -f "$(MODULE_SRC)/Makefile"
	$(MAKE) -C "$(KERNEL_SRC)" O="$(KERNEL_OUT)" \
		M="$(MODULE_SRC)" ARCH="$(KERNEL_ARCH)" -j"$(JOBS)" modules
	@mkdir -p "$(OUTPUT_DIR)"
	$(INSTALL) -D "$(KERNEL_IMAGE)" \
		"$(OUTPUT_DIR)/vmlinuz-$(KERNEL_VERSION)"
	$(INSTALL) -D "$(KERNEL_OUT)/.config" \
		"$(OUTPUT_DIR)/kernel-$(KERNEL_VERSION).config"

modules: build-kernel

kernel: build-kernel

install: userspace kernel
	@mkdir -p "$(STAGING_ROOT)/usr/lib/api" \
		"$(STAGING_ROOT)/usr/lib/hal" \
		"$(STAGING_ROOT)/usr/sbin" \
		"$(STAGING_ROOT)/usr/bin" \
		"$(STAGING_ROOT)/boot"
	cp -a $(API_LIBS) "$(STAGING_ROOT)/usr/lib/api/"
	$(INSTALL) -m 0644 "$(WHIKE_LIB)" "$(STAGING_ROOT)/usr/lib/"
	cp -a $(HAL_LIBS) "$(STAGING_ROOT)/usr/lib/hal/"
	$(INSTALL) -m 0755 $(DAEMON_BINS) "$(STAGING_ROOT)/usr/sbin/"
	$(INSTALL) -m 0755 $(TOOL_BINS) "$(STAGING_ROOT)/usr/bin/"
	$(INSTALL) -m 0644 "$(OUTPUT_DIR)/vmlinuz-$(KERNEL_VERSION)" \
		"$(STAGING_ROOT)/boot/"
	@mkdir -p "$(STAGING_ROOT)/lib/modules/$(KERNEL_VERSION)/extra"
	@find "$(MODULE_SRC)" -type f -name '*.ko' -print0 | \
		while IFS= read -r -d '' module; do \
			$(INSTALL) -m 0644 "$$module" \
				"$(STAGING_ROOT)/lib/modules/$(KERNEL_VERSION)/extra/$$(basename "$$module")"; \
		done
	@echo "[OK] Staged files: $(STAGING_ROOT)"

info:
	@echo "Project: $(PROJECT)"
	@echo "Profile: $(TARGET_PROFILE)"
	@echo "Kernel: $(KERNEL_VERSION)"
	@echo "Build jobs: $(JOBS)"
	@echo "Staging root: $(STAGING_ROOT)"

clean:
	rm -rf "$(OBJ_DIR)" "$(BIN_BUILD)" "$(LIB_BUILD)" "$(KERNEL_OUT)"

distclean: clean
	@echo "Removing downloaded sources; preserving Gentoo rootfs."
	rm -rf "$(SOURCES_DIR)"

else

ROOTFS_DIR ?= $(HOME)/gentoo-autix/rootfs
GENTOO_WORK_DIR ?= $(HOME)/gentoo-autix
PROJECT_HOST_DIR := $(abspath $(CURDIR))
JOBS ?= 4
CURL ?= curl
TAR ?= tar

STAGE_BASE_URL := https://distfiles.gentoo.org/releases/amd64/autobuilds
STAGE_LIST := $(GENTOO_WORK_DIR)/latest-stage3-amd64-systemd.txt
STAGE_ARCHIVE := $(GENTOO_WORK_DIR)/stage3-amd64-systemd.tar.xz
GOALS := $(if $(strip $(MAKECMDGOALS)),$(MAKECMDGOALS),all)
SUDO := $(shell if [ "$$(id -u)" -eq 0 ]; then echo ""; else echo "sudo"; fi)

.PHONY: all help check-tools check-gentoo prepare-gentoo userspace apis hal daemons tools download-kernel extract-kernel configure-kernel build-kernel modules kernel install install-deps sync-portage info clean distclean dispatch

all help check-tools check-gentoo prepare-gentoo userspace apis hal daemons tools download-kernel extract-kernel configure-kernel build-kernel modules kernel install install-deps sync-portage info clean distclean: dispatch

dispatch:
	@set -eu
	rootfs="$(ROOTFS_DIR)"
	work="$(GENTOO_WORK_DIR)"
	project="$(PROJECT_HOST_DIR)"
	mnt="$$rootfs/work/autumnos-mobile-main"

	mounted_dev=0
	mounted_proc=0
	mounted_sys=0
	mounted_run=0
	mounted_project=0

	cleanup() {
		if [ "$$mounted_project" -eq 1 ]; then $(SUDO) umount "$$mnt" || true; fi
		if [ "$$mounted_run" -eq 1 ]; then $(SUDO) umount -R "$$rootfs/run" || true; fi
		if [ "$$mounted_sys" -eq 1 ]; then $(SUDO) umount -R "$$rootfs/sys" || true; fi
		if [ "$$mounted_proc" -eq 1 ]; then $(SUDO) umount -R "$$rootfs/proc" || true; fi
		if [ "$$mounted_dev" -eq 1 ]; then $(SUDO) umount -R "$$rootfs/dev" || true; fi
	}
	trap cleanup EXIT

	command -v "$(CURL)" >/dev/null 2>&1 || { echo "Missing host tool: $(CURL)"; exit 1; }
	command -v "$(TAR)" >/dev/null 2>&1 || { echo "Missing host tool: $(TAR)"; exit 1; }
	$(SUDO) mkdir -p "$$work" "$$rootfs"

	if [ ! -x "$$rootfs/bin/bash" ]; then
		echo "[INFO] Fetching the latest official Gentoo amd64 systemd stage3."
		$(CURL) -fL --retry 3 "$(STAGE_BASE_URL)/latest-stage3-amd64-systemd.txt" -o "$(STAGE_LIST)"
		stage_path="$$(awk '!/^#/ && NF {print $$1; exit}' "$(STAGE_LIST)")"
		test -n "$$stage_path" || { echo "Could not determine the Gentoo stage3 archive path."; exit 1; }
		$(CURL) -fL --retry 3 "$(STAGE_BASE_URL)/$$stage_path" -o "$(STAGE_ARCHIVE)"
		$(SUDO) tar --xattrs-include='*.*' --numeric-owner -xpf "$(STAGE_ARCHIVE)" -C "$$rootfs"
		$(SUDO) cp -L /etc/resolv.conf "$$rootfs/etc/resolv.conf"
	fi

	test -x "$$rootfs/bin/bash" || { echo "Gentoo stage3 extraction failed."; exit 1; }

	$(SUDO) mkdir -p "$$rootfs/dev" "$$rootfs/proc" "$$rootfs/sys" "$$rootfs/run" "$$mnt"

	if ! mountpoint -q "$$rootfs/dev"; then
		$(SUDO) mount --rbind /dev "$$rootfs/dev"
		$(SUDO) mount --make-rslave "$$rootfs/dev"
		mounted_dev=1
	fi

	if ! mountpoint -q "$$rootfs/proc"; then
		$(SUDO) mount -t proc proc "$$rootfs/proc"
		mounted_proc=1
	fi

	if ! mountpoint -q "$$rootfs/sys"; then
		$(SUDO) mount --rbind /sys "$$rootfs/sys"
		$(SUDO) mount --make-rslave "$$rootfs/sys"
		mounted_sys=1
	fi

	if ! mountpoint -q "$$rootfs/run"; then
		$(SUDO) mount --rbind /run "$$rootfs/run"
		$(SUDO) mount --make-rslave "$$rootfs/run"
		mounted_run=1
	fi

	if ! mountpoint -q "$$mnt"; then
		$(SUDO) mount --bind "$$project" "$$mnt"
		mounted_project=1
	fi

	$(SUDO) chroot "$$rootfs" /bin/bash -lc \
		'cd /work/autumnos-mobile-main && make IN_GENTOO=1 ROOTFS_DIR=/ JOBS=$(JOBS) $(GOALS)'

endif