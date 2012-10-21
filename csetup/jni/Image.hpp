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
