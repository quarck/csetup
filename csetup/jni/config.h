#ifndef __CONFIG_H__
#define __CONFIG_H__


const char *touch_device = "/dev/input/event2";
const char *keyboard_device = "/dev/input/event3";
const char *fb_device = "/dev/graphics/fb0";


const char *usbmsslun0file = "/sys/devices/virtual/android_usb/android0/f_mass_storage/lun0/file";


const char *lettersbmp = "/system/csetup/letters.bmp";
const char *letterssmallbmp = "/system/csetup/letterssmall.bmp";

const char *cryptsetup = "/system/csetup/cryptsetup";

const char *devint = "/dev/block/mmcblk0";
const char *devsdcard = "/dev/block/mmcblk1";

const char *partmainsystem = "/dev/block/mmcblk0p22";
const char *partmaindata = "/dev/block/mmcblk0p23";
const char *partmaincache = "/dev/block/mmcblk0p24";
const char *partmaindevlog = "/dev/block/mmcblk0p28";
const char *partmainsd = "/dev/block/mmcblk1p7";

const char *partsdbootdata = "/dev/block/mmcblk1p6";
const char *partsdbootcache = "/dev/block/mmcblk1p5";
const char *partsdbootdevlog = "/dev/block/mmcblk1p3"; // using emerg's devlog - use 'emerg' passwd 
const char *partsdbootsd = "/dev/block/mmcblk1p9"; // using emerg's sd - use 'emerg' passwd
const char *partsdbootsystem = "/dev/block/mmcblk1p8";

const char *partemergdata = "/dev/block/mmcblk1p2";
const char *partemergcache = "/dev/block/mmcblk1p1";
const char *partemergdevlog = "/dev/block/mmcblk1p3";
const char *partemergsd = "/dev/block/mmcblk1p8";

const char *emergPasswd = "emerg";

const char *dmFormatCmd = "/system/bin/mke2fs -T ext4 /dev/mapper/";

#endif
