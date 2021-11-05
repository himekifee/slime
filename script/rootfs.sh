#!/bin/bash
# Modified on https://github.com/llvm/llvm-project/blob/main/lldb/scripts/lldb-test-qemu/rootfs.sh

set -e

ubuntu_source=http://archive.ubuntu.com/ubuntu

print_usage() {
  echo "Usage: $(basename $0) [options]"
  echo -e "Creates a Ubuntu root file system image.\n"
  echo -e "  --help\t\t\tDisplay this information."
  echo -e "  --arch {armhf|arm64}\t\tSelects architecture of rootfs image."
  echo -e "  --distro {bionic|focal}\tSelects Ubuntu distribution of rootfs image."
  echo -e "  --size n{K|M|G}\t\tSets size of rootfs image to n Kilo, Mega or Giga bytes."
  exit "$1"
}

invalid_arg() {
  echo "ERROR: Unrecognized argument: $1" >&2
  print_usage 1
}

# Parse options
while [[ $# -gt 0 ]]; do
  case "${END_OF_OPT}${1}" in
    --help)     print_usage 0 ;;
    --arch)     rfs_arch=$2;   shift;;
    --distro)   rfs_distro=$2; shift;;
    --size)     rfs_size=$2;   shift;;
    *)          invalid_arg "$1" ;;
  esac
  shift
done

if [ -z "$rfs_arch" ]; then
  echo "Missing architecture"
  print_usage 1
fi
if [ -z "$rfs_distro" ]; then
  echo "Missing distribution"
  print_usage 1
fi
if [ -z "$rfs_size" ]; then
  echo "Missing size"
  print_usage 1
fi

if [[ "$rfs_arch" != "arm64" && "$rfs_arch" != "armhf" && "$rfs_arch" != "amd64" ]]; then
  echo "Invalid architecture: $rfs_arch"
  print_usage 1
fi

pat='^[0-9]+[K|M|G]$'
if [[ ! $rfs_size =~ $pat ]]; then
  echo "Invalid size: $rfs_size"
  print_usage 1
fi

image_name=$rfs_distro-$rfs_arch-"rootfs"
echo "Creating $rfs_distro ($rfs_arch) root file system ..."
echo "Image name: $img_path"
echo "Image size: $rfs_size"

mkdir ../vm
img_path=../vm/$image_name.img
qemu-img create $img_path $rfs_size

mkfs.ext4 $img_path
mkdir ../vm/$image_name.dir
chroot_path=../vm/$image_name.dir
sudo mount -o loop $img_path $chroot_path

sudo debootstrap --arch $rfs_arch $rfs_distro $chroot_path $ubuntu_source

sudo chroot $chroot_path /usr/sbin/locale-gen en_US.UTF-8

sudo chroot $chroot_path sed -i \
's/main/main restricted multiverse universe/g' /etc/apt/sources.list

sudo chroot $chroot_path sed -i '$ a\nameserver 8.8.8.8' /etc/resolv.conf

sudo chroot $chroot_path apt update
sudo chroot $chroot_path apt -y install ssh bash-completion
sudo chroot $chroot_path /usr/sbin/adduser --gecos "" $USER
sudo chroot $chroot_path /usr/sbin/adduser $USER sudo
sudo umount $chroot_path
rmdir $chroot_path
