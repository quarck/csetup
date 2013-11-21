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

#ifndef __EMERG_RAMDISK_HPP__
#define __EMERG_RAMDISK_HPP__


#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/loop.h>
#include <sys/errno.h>

int createramdisk(const char *mp, const char *loop, int loopNr)
{
	mkdir("/dev/mapper", 0755); // if does not exists

	// mount -t tmpfs -o nosuid,nodev tmpfs ./mp 
	int ret = mount("tmpfs", mp, "tmpfs", MS_NOSUID | MS_NODEV, "");

	if ( ret == 0 ) 
	{

		char namebuf[1024];
		char buf[4096];

		snprintf(namebuf, sizeof(namebuf), "%s/file.img", mp);

		int imgfd = open(namebuf, O_RDWR|O_CREAT, 0600);

		if ( imgfd != -1 ) 
		{
			memset(buf, 0, sizeof(buf));

			for (int i=0; i<1024 * 64; i++ ) 
			{
				if ( write(imgfd, buf, sizeof(buf)) != sizeof(buf))
					break;
			}

			struct stat statbuf;
			
			if ( stat(loop, &statbuf) != 0 || !S_ISBLK(statbuf.st_mode)) 
			{
				mknod(loop, S_IFBLK|0644, makedev(7, loopNr));
			}

			int dfd = open(loop, O_RDWR);

			if ( dfd != -1 ) 
			{
				loop_info64 loopinfo;

				ret = ioctl(dfd, LOOP_GET_STATUS64, &loopinfo);

				if ( ret != 0 && errno == ENXIO ) 
				{
					memset(&loopinfo, 0, sizeof(loopinfo));

					strncpy((char *)loopinfo.lo_file_name, namebuf, LO_NAME_SIZE);

					loopinfo.lo_offset = 0;
					/* Associate free loop device with file.  */

					if (ioctl(dfd, LOOP_SET_FD, imgfd) == 0) 
					{
						if (ioctl(dfd, LOOP_SET_STATUS64, &loopinfo) == 0)
							ret = 0;
						else
							ioctl(dfd, LOOP_CLR_FD, 0);
					}

				}
			}
			else 
			{
				ret = -1;
			}

			close(imgfd);
		}
		else
		{
			ret = -1;
		}
	}



	return ret;
}


#endif
