#!/bin/bash
#will make an image in ~/git/LLD/linux_os/qemu/myos.img and run it with qemu

rm -f ~/git/LLD/linux_os/qemu/myos.img
qemu-img create ~/git/LLD/linux_os/qemu/myos.img 512M
mkfs.ext4 ~/git/LLD/linux_os/qemu/myos.img
mkdir -p ~/git/LLD/linux_os/qemu/mnt
sudo mount ~/git/LLD/linux_os/qemu/myos.img ~/git/LLD/linux_os/qemu/mnt
sudo cp -r /mnt/myos/* ~/git/LLD/linux_os/qemu/mnt
sudo umount ~/git/LLD/linux_os/qemu/mnt

qemu-system-x86_64 -kernel /boot/vmlinuz-linux -initrd /boot/initramfs-linux.img -m 512m -append "root=/dev/sda" -hda ~/git/LLD/linux_os/qemu/myos.img
