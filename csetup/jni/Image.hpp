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

#ifndef __IMAGE_HPP__
#define __IMAGE_HPP__

#include <sys/types.h>
#include <sys/stat.h>

#include <cstring>

#include "UI.hpp"

class Image 
{
private:
	unsigned char	*m_imageBuffer;	
	size_t		m_imageBufferSize;
	
	unsigned	m_width;
	unsigned	m_height;
	
	size_t		m_bitsPerPixel;
	size_t		m_bytesPerPixel;

	size_t		m_bytesPerRow;

	Image() {}
	Image(const Image& img) {}

public:
	inline unsigned width() const { return m_width; }
	
	inline unsigned height() const { return m_height; }
	

	inline unsigned char *buffer() const { return m_imageBuffer; }
	
	inline size_t bufferSize() const { return m_imageBufferSize; }


	inline size_t offsetForPosition(unsigned x, unsigned y) const
	{
		return y * m_bytesPerRow  +  x * m_bytesPerPixel;
	}
	
	inline size_t bitsPerPixel() const { return m_bitsPerPixel; }
	
	inline size_t bytesPerRow() const { return m_bytesPerRow; }

	inline size_t bytesPerPixel() const { return m_bytesPerPixel; }


	inline Image(	unsigned width,
			unsigned height, 
			size_t bitsPerPixel,
			size_t bytesPerRow 
		)
		: m_imageBuffer(NULL)
		, m_width(width)
		, m_height(height)
		, m_bitsPerPixel(bitsPerPixel)
		, m_bytesPerRow(bytesPerRow)
		, m_bytesPerPixel(bitsPerPixel/8)
	{
		m_imageBufferSize = bytesPerRow * m_height;
		m_imageBuffer = new unsigned char[m_imageBufferSize];
	}
	
	inline ~Image()
	{
		delete [] m_imageBuffer;
	}
};


#endif
