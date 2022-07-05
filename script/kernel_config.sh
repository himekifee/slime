#!/bin/sh
mv .config .config.bak
make defconfig

# Debug options
./scripts/config -e CONFIG_EXPERT -d CONFIG_COMPILE_TEST -e CONFIG_DEBUG_KERNEL -e CONFIG_DEBUG_INFO -e CONFIG_GDB_SCRIPTS -e CONFIG_HYPERVISOR_GUEST -e CONFIG_PARAVIRT -e CONFIG_PARAVIRT_XXL -e CONFIG_PARAVIRT_DEBUG -e CONFIG_PARAVIRT_SPINLOCKS -e CONFIG_X86_X2APIC -d CONFIG_DEBUG_INFO_REDUCED

# Boot requirements
./scripts/config -e CONFIG_VIRTIO_BLK -e CONFIG_VIRTIO_PCI -e CONFIG_EXT4_FS

# Network
./scripts/config -e CONFIG_VIRTIO_NET

# Misc
./scripts/config -e CONFIG_USER_NS

# Monolithic kernel
./scripts/config -d CONFIG_MODULES

make olddefconfig
