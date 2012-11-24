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

#ifndef __BMP_HPP__
#define __BMP_HPP__

#include <sys/types.h>

#include "Image.hpp"

namespace bmp
{

// nested header structure decl
struct BmpHeader
{
	u_int8_t	signature[2];		// $00-$01  ASCII 2-byte "BM" bitmap identifier. 
	u_int32_t	length;			// $02-$05  Total length of bitmap file in bytes. 
	u_int32_t	reseved1;		// $06-$09  Reserved, possibly for image id or revision. 
	u_int32_t	pixel_data_offset;	// $0A-$0D  Offset to start of actual pixel data. 
	
	u_int32_t 	hdr_size;		// $0A-$11  Size of data header, usually 40 bytes. 
	u_int32_t	width;			// $12-$15  Width of bitmap in pixels. 
	u_int32_t	height;			// $16-$19  Height of bitmap in pixels. 
	u_int16_t	num_color_planes;	// $1A-$1B  Number of color planes. Usually 01 
	u_int16_t	bits_per_pixel;		// $1C-$1D  Number of bits per pixel. Sets color mode. 
	u_int32_t	compression_mode;	// $1E-$21  Non-lossy compression mode in use, 0 - None 
	u_int32_t	data_size;		// $22-$25  Size of stored pixel data 
	u_int32_t	width_resolution;	// $26-$29  Width resolution in pixels per meter 
	u_int32_t	height_resolution;	// $2A-$2D  Height resolution in pixels per meter 
	u_int32_t	colors_used;		// $2E-$31  Number of colors actually used. 
	u_int32_t	important_colors;	// $32-$35  Number of important colors 
	
	static inline unsigned align4(unsigned i)
	{
	}
			
	inline u_int32_t getWidth()
	{
		return width;
	}
	
	inline u_int32_t getHeight()
	{
		return height;
	}

	inline u_int16_t getBitsPerPixel()
	{
		return bits_per_pixel;
	}
	
	inline u_int32_t getBytesInRow()
	{
		u_int32_t v = getBitsPerPixel() * getWidth() / 8;
		return 	(v + 3) & ~3;
	}
	
public:
	//
	// stub constructor
	//
	BmpHeader()
	{
	}
	

} __attribute__((__packed__));



inline Image* readImage(const char *fileName)
{
	Image* img = NULL;

	int fd = open(fileName, O_RDONLY);

	if ( fd == -1 ) 
	{
		return NULL;
	}

	BmpHeader hdr;

	if ( read(fd, &hdr, sizeof(hdr)) != sizeof(hdr) )
	{
		close(fd);
		return NULL;
	}

	int w = hdr.getWidth();
	int h = hdr.getHeight();

	img = new Image(  
			w, 
			h, 
			hdr.getBitsPerPixel(),
			hdr.getBytesInRow()
		);


	if ( img )
	{
		unsigned char *buf = img->buffer();
		
		int bytesPerRow = hdr.getBytesInRow();
		
		for (int k = h-1; k >= 0; k --)
		{
			if ( read(
				fd, 
				buf + img->offsetForPosition(0, k), 
				bytesPerRow
			      ) != bytesPerRow 
			   ) 
			{
				delete img;
				img = NULL;
			}
		}
	}

	close(fd);

	printf("returning img %p\n", img );
	if (img) 
	{
		printf("%d %d\n", img->width(), img->height() );
	}

	return img;
}

};

#endif
