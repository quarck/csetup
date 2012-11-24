/*
 * Copyright (c) 2012, Sergey Parshin, qrck@mail.ru
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
