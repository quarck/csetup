cd initrd
find . | cpio -o -H newc | gzip > ../ramdisk
